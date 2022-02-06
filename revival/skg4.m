#! /usr/bin/octave -qf
# -*-octave-*-
# SKG model DOI: 10.1021/jp9832721
# Based on Eqs E6-E8 and simulate the dynamics by varying initial [BrO3-]
# and [BrCHD], both of which decay as a function of time.
#
# Additionally this implementation includes linear decay of [BrO3-].
# [BrO3-] = [BrO3-]_0 - T * t;

global galpha = 1000000;
global gbeta = 77.4;
global ggamma = 1102;
global gdelta = 2719;
global gf = 0.776;
global BrO30 = 0.2;
global BrCHD0 = 0.022;
global kBrO3 = 0;
global kBrCHD = 0;
global k2 =  2500000;
global k14 =  5.0000e-05;
global k17 =  0.02;
global BrCHD = 0.02;
global kdp = 280; # kd'
global k5 = 170000;
global k4 = 2000000;
global k4r = 100000000; # k_{-4}
global H = 1.29;

arg_list = argv();

# Command-line arguments are mandatory
if (nargin > 3)
  BrO30 = str2num(arg_list{1});
  BrCHD0 = str2num(arg_list{2});
  kBrO3 = str2num(arg_list{3});
  kBrCHD = str2num(arg_list{4});
else
  quit(1);
endif

if (5 == nargin)
  t_end = str2num(arg_list{5});
else
  t_end = 25;
endif

printf("BrO30 = %f, BrCHD0 = %f, kBrO3 = %f, kBrCHD = %f\n",
	   BrO30, BrCHD0, kBrO3, kBrCHD);

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
  global BrO30;
  global BrCHD0;
  global kBrO3;
  global kBrCHD;
  global H;

  BrO3 = axz(t); #BrO30 - kBrO3 * t;
  BrCHD = bxz(t); #BrCHD0 - kBrCHD * t;

  xdot = zeros(3, 1);

  galpha = k2 * k14 * BrCHD / (k17 * k17 * BrO3 * BrO3);
  # beta has no dependency on [BrO3-] or [BrCHD]
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

function r = axz(x)
  global BrO30 kBrO3;

  #r = BrO30 - kBrO3 * x;

  #if r < 0.04
	#r = 0.04;
  #endif

  # 2) Same as 0181121_skg4.jpg  but both [BrO3-] and [BrCHD] decay 
  # exponentially: C(t) =C(0)*EXP(-0.2*t).

  #r = 0.001 + BrO30 * exp(-0.2 * x);
  r = 0.001 + BrO30 * exp(-kBrO3 * x);
  #r = 0.04;
endfunction

function r = bxz(x)
  global BrCHD0 kBrCHD;

  #r = BrCHD0 - kBrCHD * x; 

   #if r < 0.001
   #	r = 0.001;
   #endif

  #  (a) [BrCHD] = 0.022 (constant)
  # r = 0.022;
  # (b) [BrCHD] = 0 
  # r = 0.001;
  # (c) (or constant to be 0.0022).
  # r = 0.0022;

  # 2) Same as 0181121_skg4.jpg  but both [BrO3-] and [BrCHD] decay 
  # exponentially: C(t) =C(0)*EXP(-0.2*t).

  #r = 0.001 + BrCHD0 * exp(-0.6 * x);
  r = 0.001 + BrCHD0 * exp(-kBrCHD * x);

# 3) Same as 2) but [BrCHD] is set constant as 1). 

  #  (a) [BrCHD] = 0.022 (constant)
  #r = 0.022;
  # (b) [BrCHD] = 0 
  #r = 0.001;
  # (c) (or constant to be 0.0022).
  #r = 0.0022;


endfunction


# Initial conditions
#1 HBrO2
x1 = 0.0007245927973977255 ;
#2 Br-
x2 = 0.001422769822454512 ;
#3 H2Q
x3 = 0.03193424249332828 ;

x0 = [x1; x2; x3];

t = linspace (0, t_end, 10000);

y = lsode ("f", x0, t);

plot (t, y);

xlabel("\\tau");
ylabel("concentration");
title(cstrcat(
			  "SKG4 model: ",
			  " f=", num2str(gf),
			  " [BrO_3^-]_0=", num2str(BrO30),
			  " [BrCHD]_0=", num2str(BrCHD0),
			  " k_{BrO3}=", num2str(kBrO3),
			  " k_{BrCHD}=", num2str(kBrCHD)
			  )
	  );
legend("[HBrO_2]", "[Br^-]", "[H_2Q]");

print -djpg skg4.jpg
print skg4.pdf
#quit();

newplot();

hold on;
#plot(y(:,1),y(:,3), "color", "black");
plot(t, axz(t), "color", "blue");
plot(t, bxz(t), "color", "red");

#xlim([0, 0.015]);
ylim([0, 0.25]);

xlabel("\\tau");
ylabel("concentration");
title(cstrcat(
			  "SKG4 model: ",
			  " f=", num2str(gf),
			  " [BrO_3^-]_0=", num2str(BrO30),
			  " [BrCHD]_0=", num2str(BrCHD0),
			  " k_{BrO3}=", num2str(kBrO3),
			  " k_{BrCHD}=", num2str(kBrCHD)
			  )
	  );

legend("[BrO_3^-]", "[BrCHD]");

print -djpg skg4x.jpg
print skg4x.pdf

# d = [rot90(t, -1), y, rot90(null_x, -1), rot90(null_z, -1)];
d = [rot90(t, -1), y] ;

save skg4.mat d;

d = [rot90(t, -1), rot90(axz(t), -1), rot90(bxz(t), -1)] ;

save skg4x.mat d ;


quit(0);



