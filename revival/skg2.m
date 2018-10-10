#! /usr/bin/octave -qf
# -*-octave-*-
# SKG model DOI: 10.1021/jp9832721
# Eqs E6-E8 modified to simulation the consumption of substrate BrO3-


global galpha = 1000000;
global gbeta = 77.4;
global ggamma = 1102;
global gdelta = 2719;
global gf = 0.776;
global BrO30 = 0.08;
global T = 0;
global k2 =  2500000;
global k14 =  5.0000e-05;
global k17 =  0.02;
global BrCHD = 0.02;
global kdp = 280; # kd'
global k5 = 170000;
global k4 = 2000000;
global k4r = 100000000; # k_{-4}
global H = 1.29;

if (1 == nargin)
  arg_list = argv();
  BrO30 = str2num(arg_list{1});
endif

function xdot = f (x, t)
  global galpha;
  global gbeta;
  global ggamma;
  global gdelta;
  global gf;
  global k2;
  global k14;
  global k17;
  global kdp;
  global k5;
  global k4;
  global k4r;
  global BrCHD;
  global T;
  global BrO30
  global H;

  BrO3 = BrO30 - T * t; # FIXME check BrO3 > 0

  xdot = zeros (3, 1);

  galpha = k2 * k14 * BrCHD / (k17 * k17 * BrO3 * BrO3);
  # beta has no dependency on [BrO3-]
  ggamma = kdp * realsqrt(k14) * realsqrt(BrCHD) / (realpow(k17, 3/2) * realsqrt(H) * BrO3);
  gdelta = 2 * k5 * k4 * k14 * BrCHD / (k4r * k17 * k17 * BrO3 * BrO3);

  # printf("a = %f\n", galpha);
  # printf("b = %f\n", gbeta);
  # printf("g = %f\n", ggamma);
  # printf("d = %f\n", gdelta);

  # quit();

  xdot(1) = x(2) * (gbeta - galpha * x(1)) + x(3) * (ggamma * realsqrt(x(1)) + 1.0) - gdelta * x(1) * x(1);
  xdot(2) = 1.0 - x(2) * (gbeta + galpha * x(1));
  xdot(3) = gf - x(3) * (ggamma * realsqrt(x(1)) + 1.0); 

endfunction



#1 HBrO2
x1 = 0.01;
#2 Br-
x2 = 0;
#3 H2Q
x3 = 0.01;

x0 = [x1; x2; x3];

# time = kf*t
t = linspace (0, 1, 10000);

y = lsode ("f", x0, t);


plot (t, y);

xlabel("\\tau");
ylabel("concentration");
title(cstrcat("SKG2 model: ",
			  " f=", num2str(gf), 
			  " [BrO_3^-]_0=", num2str(BrO30))
#			  " T=", num2str(T))
	  );
legend("HBrO_2", "Br^-", "H_2Q");

print -djpg skg2.jpg

d = [rot90(t, -1), y];

save plot.mat d;

quit(0);



