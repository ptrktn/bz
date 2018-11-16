#! /usr/bin/octave -qf
# -*-octave-*-
# SKG model DOI: 10.1021/jp9832721
# Based on Eqs E6-E8 and simulate the dynamics by varying initial [BrO3-]
#
# Additionally this implementation includes linear decay of [BrO3-].
# [BrO3-] = [BrO3-]_0 - T * t;

global galpha = 1000000;
global gbeta = 77.4;
global ggamma = 1102;
global gdelta = 2719;
global gf = 0.776;
global BrO30 = 0.08;
# T = 0 simulates [Br03-] is not consumed
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

# The first command-line argument, if given, is initial [BrO3-]
if (1 == nargin)
  arg_list = argv();
  BrO30 = str2num(arg_list{1});
endif

# Eqs E6-E8
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

  xdot(1) = x(2) * (gbeta - galpha * x(1)) + x(3) * (ggamma * realsqrt(x(1)) + 1.0) - gdelta * x(1) * x(1);
  xdot(2) = 1.0 - x(2) * (gbeta + galpha * x(1));
  xdot(3) = gf - x(3) * (ggamma * realsqrt(x(1)) + 1.0); 

endfunction

# Initial conditions
#1 HBrO2
x1 = 0.0007245927973977255 ;
#2 Br-
x2 = 0.001422769822454512 ;
#3 H2Q
x3 = 0.03193424249332828 ;

x0 = [x1; x2; x3];

# time = kf*t
#t = linspace (0, 1, 10000);
t = linspace (0, 5, 10000);

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

# E9
function r = axz(x)
  global galpha gbeta ggamma gdelta gf;

  v = galpha/gbeta;

  r = (gdelta * x * x - (1 - v * x)/(1 + v * x))/(ggamma * realsqrt(x) + 1); 

endfunction

# E10
function r = bxz(x)
  global ggamma gf;

  r = gf / (ggamma * realsqrt(x) + 1);
endfunction


newplot();

xsteps = 1000;
xmax = 1.1 * max(y(:,1));
xh = xmax/xsteps;
xi = 1;

for i=1:xsteps
  xx = xh * i;
  yy = axz(xx);

  # a(x,z) may become negative with small x values - do not plot those
  if (yy > 0)
	xxi(xi) = xx;
	null_x(xi) = yy;
	null_z(xi) = bxz(xx);
	xi = xi + 1;
  endif

end

hold on;
plot(y(:,1),y(:,3), "color", "black");
plot(xxi, null_x, "color", "blue");
plot(xxi, null_z, "color", "red");

xlim([0, 0.015]);
ylim([0, 0.04]);

title(cstrcat("SKG2 model: ",
			  " f=", num2str(gf), 
			  " [BrO_3^-]_0=", num2str(BrO30))
	  );
xlabel("x");
ylabel("z");
legend("x-z", "a(x,z)", "b(x,z)");

print -djpg skg2pp.jpg

quit(0);



