#! /usr/bin/octave -qf
# -*-octave-*-
# SK model DOI: 10.1021/jp9832721
# Szalai and Koros, J. Phys. Chem. A 1998, 102, 6892-6897
#
#

more off

global n kf kr;

n = 0;

# small initial concentration for intermediates     
int0 = 0; 
   
x0 = NaN(17, 1);
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
x0(14) = 0.1 ; 
# x(15): CHDE
x0(15) = int0;
# x(16): BrCHD
x0(16) = 0.0205;
# x(17): CHED
x0(17) = 0.00425;

# forward reaction rates
kf = zeros(19, 1);
# reverse reaction rates
kr = zeros(19, 1);

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


source("sk_jac.m");


function xdot = f (x, t)
  global n kf kr;
  
  xdot = zeros(17, 1);
  
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

  
  # Table 3
  # x(1): Br-
  xdot(1) =  - fkinet(1) + rkinet(1) ;         # R1
  xdot(1) += - fkinet(2) + rkinet(2) ;         # R2
  xdot(1) += - fkinet(3) + rkinet(3) ;         # R3
  xdot(1) +=   fkinet(12) ;                    # R12
  xdot(1) +=   fkinet(14) ;                    # R14
  xdot(1) +=   2 *  fkinet(16) ;               # R16
  xdot(1) +=   fkinet(18) ;                    # R18
  # x(2): HOBr
  xdot(2) =  - fkinet(1) + rkinet(1) ;         # R1
  xdot(2) +=   2 * fkinet(2) - rkinet(2) ;     # R2
  xdot(2) +=   fkinet(3) - rkinet(3) ;         # R3
  xdot(2) +=   fkinet(5) ;                     # R5
  xdot(2) += - fkinet(13) ;                    # R13
  xdot(2) += - fkinet(18) ;                    # R18  
  # x(3): H+
  xdot(3) =  - fkinet(1) + rkinet(1) ;         # R1
  xdot(3) += - fkinet(2) + rkinet(2) ;         # R2
  xdot(3) += - 2 * fkinet(3) + 2 * rkinet(3) ; # R3
  xdot(3) += - rkinet(4) + rkinet(4) ;         # R4
  xdot(3) +=   2 * fkinet(5) ;                 # R5
  xdot(3) += - fkinet(6) - rkinet(6) ;         # R6
  xdot(3) += - fkinet(11) - rkinet(11) ;       # R11
  xdot(3) +=   fkinet(12) ;                    # R12
  xdot(3) +=   fkinet(14) ;                    # R14
  # CHED + H+ -> H2Q + H+
  xdot(3) +=   fkinet(15)  ;                   # R15 CHECK
  xdot(3) +=   2 * fkinet(16) ;                # R16
  xdot(3) += - fkinet(17) ;                    # R17
  xdot(3) +=   fkinet(18) ;                    # R18
  xdot(3) += - fkinet(19) ;                    # R19
  # x(4): Br2
  xdot(4) =    fkinet(1) - rkinet(1) ;         # R1
  xdot(4) += - fkinet(12) ;                    # R12
  xdot(4) += - fkinet(16) ;                    # R16
  # x(5): H2O
  xdot(5) =    fkinet(1) - rkinet(1) ;         # R1
  xdot(5) +=   fkinet(6) - rkinet(6) ;         # R6
  xdot(5) +=   fkinet(13) ;                    # R13
  xdot(5) +=   fkinet(17) ;                    # R17
  xdot(5) +=   fkinet(18) ;                    # R18
  xdot(5) +=   fkinet(19) ;                    # R19
  # x(6): HBrO2
  xdot(6) =  - fkinet(2) + rkinet(2) ;         # R2
  xdot(6) +=   fkinet(3) - rkinet(3) ;         # R3
  xdot(6) += - fkinet(4) + rkinet(4) ;         # R4
  xdot(6) += - fkinet(5) ;                     # R5
  xdot(6) += - fkinet(6) + rkinet(6) ;         # R6
  xdot(6) +=   fkinet(8) ;                     # R8
  xdot(6) +=   fkinet(9) ;                     # R9
  xdot(6) +=   fkinet(17) ;                    # R17
  xdot(6) +=   fkinet(19) ;                    # R19
  # x(7): BrO3-
  xdot(7) =   - fkinet(3) + rkinet(3) ;        # R3
  xdot(7) +=    fkinet(5) ;                    # R5
  xdot(7) +=  - fkinet(6) + rkinet(6) ;        # R6
  xdot(7) +=  - fkinet(17) ;                   # R17
  xdot(7) +=  - fkinet(19) ;                   # R19
  # x(8): H2BrO2+
  xdot(8) =    fkinet(4) - rkinet(4) ;         # R4
  xdot(8) += - fkinet(5) ;                     # R5
  # x(9): Br2O4
  xdot(9) =    fkinet(6) - rkinet(6) ;         # R6
  xdot(9) += - fkinet(7) + rkinet(7) ;         # R7
  # x(10): BrO2*
  xdot(10) =    2 * fkinet(7) - 2 * rkinet(7) ; # R7
  xdot(10) += - fkinet(8) ;                    # R8
  xdot(10) += - fkinet(9) ;                    # R9
  # x(11): H2Q
  xdot(11) =   - fkinet(8) ;                   # R8
  xdot(11) +=    fkinet(10) - rkinet(10) ;     # R10
  xdot(11) +=    fkinet(15) ;                  # R15
  xdot(11) +=  - fkinet(16) ;                  # R16
  xdot(11) +=  - fkinet(17) ;                  # R17
  xdot(11) +=  - fkinet(18) ;                  # R18
  xdot(11) +=    fkinet(19) ;                  # R19
  # x(12): HQ*
  xdot(12) =    fkinet(8) ;                    # R8
  xdot(12) += - fkinet(9) ;                    # R9
  xdot(12) += - 2 * fkinet(10) + 2 * rkinet(10) ;  # R10
  # x(13): Q
  xdot(13) =    fkinet(9) ;                    # R9
  xdot(13) +=   fkinet(10) - rkinet(10) ;      # R10
  xdot(13) +=   fkinet(16) ;                   # R16
  xdot(13) +=   fkinet(17) ;                   # R17
  xdot(13) +=   fkinet(18) ;                   # R18
  # x(14): CHD
  xdot(14) =  - fkinet(11) + rkinet(11) ;      # R11
  xdot(14) += - fkinet(19) ;                   # R19
  # x(15): CHDE
  xdot(15) =    fkinet(11) - rkinet(11) ;      # R11
  xdot(15) += - fkinet(12) ;                   # R12
  xdot(15) += - fkinet(13) ;                   # R13
  # x(16): BrCHD
  xdot(16) =    fkinet(12) ;                   # R12
  xdot(16) +=   fkinet(13) ;                   # R13
  xdot(16) += - fkinet(14) ;                   # R14
  # x(17): CHED
  xdot(17) =    fkinet(14) ;                   # R14
  xdot(17) += - fkinet(15) ;                   # R15
 
  n += 1;

  if 0 == mod(n, 10000)
	disp(t)
  endif

endfunction

save sk_x0.mat x0;
save sk_kf.mat kf;
save sk_kr.mat kr;

usej = 1;
useplot = 0;
t1 = 60000;

tsteps = 100 * t1;
t = linspace (0, t1, tsteps);

lsode_options("integration method", "stiff");
lsode_options("absolute tolerance", 1e-5);
lsode_options("relative tolerance", 1e-5);
lsode_options("maximum step size", 1e-3);
lsode_options("step limit", 1000000);

lsode_options

if 0 == usej
  [y, istate, msg] = lsode ("f", x0, t);  
else
  [y, istate, msg] = lsode ({@f, @jac}, x0, t);
endif

istate
msg

if 1 == useplot

h1 = figure();

plot (t, y);

xlabel("time");
ylabel("concentration");

title(cstrcat(
			  "SK model ",
        ""
			  )
	  );

#legend("[HBrO_2]", "[Br^-]", "[H_2Q]");

print(h1, "sk.jpg", "-djpg");

else
  
  d = [rot90(t, -1), y];
  # time H2Q HBrO2 Br-
  #d = [rot90(t, -1), y(:, 11), y(:, 6), y(:, 1)];
  save sk.mat d;
  
endif



