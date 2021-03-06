#! /usr/bin/octave -qf
# -*-octave-*-
# SKG model DOI: 10.1021/jp9832721
# Eqs E6-E8


global galpha = 1000000;
global gbeta = 77.4;
global ggamma = 1102;
global gdelta = 2719;
global gf = 0.776;

if (1 == nargin)
  arg_list = argv();
  gf = str2num(arg_list{1});
endif

function xdot = f (x, t)
  global galpha;
  global gbeta;
  global ggamma;
  global gdelta;
  global gf;

  xdot = zeros (3, 1);

  # alpha = ka * kc / realpow(kf, 2);
  # beta = kb / kf;
  # delta = 2 * kc * k3 / realpow(kf, 2);
  # gamma = kd * realsqrt(ke) / realpow(kf, 3/2);

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
title(cstrcat("SKG model: \\alpha=", num2str(galpha),
			  " \\beta=", num2str(gbeta),
			  " \\gamma=", num2str(ggamma),
			  " \\delta=", num2str(gdelta),
			  " f=", num2str(gf))
	  );
legend("HBrO_2", "Br^-", "H_2Q");

print -djpg skg.jpg

d = [rot90(t, -1), y];

save plot.mat d;

quit(0);



