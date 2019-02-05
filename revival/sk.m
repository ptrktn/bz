#! /usr/bin/octave -qf
# -*-octave-*-
# SK model DOI: 10.1021/jp9832721
# Szalai and Koros, J. Phys. Chem. A 1998, 102, 6892-6897
#
#

global kf kr;

# small initial concentration for intermediates     
int0 = 0.1/10000; 
   
x0 = zeros(17, 1);
# x(1): Br-
x0(1) = int0;
# x(2): HOBr
x0(2) = int0;
# x(3): H+
x0(3) = 1.29; # 2.0 M???
# x(4): Br2
x0(4) = 10^(-3);
# x(5): H2O
x0(5) = 55;
# x(6): HBrO2
x0(6) = int0;
# x(7): BrO3-
x0(7) = 8 * 10^(-3);
# x(8): H2BrO2+
x0(8) = int0;
# x(9): Br2O4
x0(9) = int0;
# x(10): BrO2*
x0(10) = int0;
# x(11): H2Q
x0(11) = 5 * 10^(-5);
# x(12): HQ*
x0(12) = int0;
# x(13): Q
x0(13) = 0.0182;
# x(14): CHD
x0(14) = 0.1;
# x(15): CHDE
x0(15) = int0;
# x(16): BrCHD
x0(16) = 0.0205;
# x(17): CHED
x0(17) = 0.00425;

# forward reaction rates
kf = NaN(17, 1);
# reverse reaction rates
kr = NaN(17, 1);

kf(1) = 8 * 10^9;
kr(1) = 80;
kf(2) = 2.5 * 10^6;
kr(2) = 2 * 10^(-5);
kf(3) = 1.2;
kr(3) = 3.2;
kf(4) = 2 * 10^6;
kr(4) = 10^8;
kf(5) = 1.7 * 10^5;
kf(6) = 48;
kr(6) = 3.2 * 10^3;
kf(7) = 7.5 * 10^4;
kr(7) = 1.4 * 10^9;
kf(8) = 8 * 10^5;
kf(9) = 8 * 10^9;
kf(10) = 8.8 * 10^8;
kr(10) = 7.7 * 10^(-4);
kf(11) = 2.13 * 10^(-4);
kr(11) = 5.2 * 10^2;
kf(12) = 2.8 * 10^9;
kf(13) = 2.8 * 10^9;
kf(14) = 5 * 10^(-5);
kf(15) = 1.94 * 10^(-4);
kf(16) = 3 * 10^4;
kf(17) = 2 * 10^(-2);
kf(18) = 6 * 10^5;
kf(19) = 10^(-5);




function xdot = f (x, t)
  global kf kr;
  
  fkinet(1) = kf(1) * x(1) * x(2) * x(3);
  rkinet(1) = kr(1) * x(4) * x(5);
  fkinet(2) = kf(2) * x(1) * x(6) * x(3);
  rkinet(2) = kr(2) * x(2) * x(2);
  fkinet(3) = kf(3) * x(1) * x(7) * x(3) * x(3);
  rkinet(3) = kr(3) * x(2) * x(6);
  fkinet(4) = kf(4) * x(6) * x(3);
  rkinet(4) = kr(4) * x(8);
  fkinet(5) = kf(5) * x(6) * x(8);
  fkinet(6) = kf(6) * x(6) * x(7) * x(3);
  rkinet(6) = kr(6) * x(9) * x(5);
  fkinet(7) = kf(7) * x(9);
  rkinet(7) = kr(7) * x(10) * x(10);
  fkinet(8) = kf(8) * x(11) * x(10);
  fkinet(9) = kf(9) * x(12) * x(10);
  fkinet(10) = kf(10) * x(12) * x(12);
  rkinet(10) = kr(10) * x(11) * x(13);
  fkinet(11) = kf(11) * x(14) * x(3);
  rkinet(11) = kr(11) * x(15) * x(3);
  fkinet(12) = kf(12) * x(15) * x(4);
  fkinet(13) = kf(13) * x(15) * x(2);
  fkinet(14) = kf(14) * x(16);
  fkinet(15) = kf(15) * x(17) * x(3);
  fkinet(16) = kf(16) * x(11) * x(4);
  fkinet(17) = kf(17) * x(11) * x(3) * x(7);
  fkinet(18) = kf(18) * x(11) * x(2);
  fkinet(19) = kf(19) * x(14) * x(7) * x(3); 
  
  xdot = zeros(17, 1);

  # FIXME rate equations
# x(1): Br-

# x(2): HOBr
# x(3): H+
# x(4): Br2
# x(5): H2O
# x(6): HBrO2
# x(7): BrO3-
# x(8): H2BrO2+
# x(9): Br2O4
# x(10): BrO2*
# x(11): H2Q
# x(12): HQ*
# x(13): Q
# x(14): CHD
# x(15): CHDE
# x(16): BrCHD
# x(17): CHED

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
  #r = 0.001 + BrO30 * exp(-kBrO3 * x);
  r = 0.0005 + BrO30 * exp(-kBrO3 * x);
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

# time = kf*t
#t = linspace (0, 1, 10000);
#t = linspace (0, 5, 10000);
t = linspace (0, 15, 10000);
#t = linspace (0, 1, 1000);

y = lsode ("f", x0, t);

h1 = figure();
#plot (t, axz(t));
#subplot(2, 1, 1);
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

#print -djpg skg4.jpg
#quit();


#subplot(2, 1, 2);
h2 = figure();

hold on;
#plot(y(:,1),y(:,3), "color", "black");
plot(t, axz(t), "color", "blue");
plot(t, bxz(t), "color", "red");

#xlim([0, 0.015]);
#ylim([0, 0.04]);

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

#print -djpg -color skg4.jpg

#d = [rot90(t, -1), y, rot90(null_x, -1), rot90(null_z, -1)];

#save plot.mat d;


#quit(0);

#endif

print(h1, "skg4a.jpg", "-djpg");

print(h2, "skg4b.jpg", "-djpg");

