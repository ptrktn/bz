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
  
  xdot = NaN(17, 1);

  # Table 3
  # x(1): Br-
  xdot(1) = - fkinet(1) + rkinet(1)          # R1
            - fkinet(2) + rkinet(2)          # R2
            - fkinet(3) + rkinet(3)          # R3
            + fkinet(12)                     # R12
            + fkinet(14)                     # R14
            + 2 *  fkinet(16)                # R16
            + fkinet(18);                    # R18
  # x(2): HOBr
  xdot(2) = - fkinet(1) + rkinet(1)          # R1
            + 2 * fkinet(2) - rkinet(2)      # R2
            + fkinet(3) - rkinet(3)          # R3
            + fkinet(5)                      # R5
            - fkinet(18);                    # R18  
  # x(3): H+
  xdot(3) = - fkinet(1) + rkinet(1)          # R1
            - fkinet(2) + rkinet(2)          # R2
            - fkinet(3) + rkinet(3)          # R3
            - rkinet(4) + rkinet(4)          # R4
            + 2 * fkinet(5)                  # R5
            - fkinet(6) - rkinet(6)          # R6
            - fkinet(11) - rkinet(11)        # R11
            + fkinet(12)                     # R12
            + fkinet(14)                     # R14
            - fkinet(15)                     # R15
            + 2 * fkinet(16)                 # R16
            - rkinet(17)                     # R17
            + fkinet(18)                     # R18
            - fkinet(19);                    # R19
  # x(4): Br2
  xdot(4) = + fkinet(1) - rkinet(1)          # R1
            - fkinet(12)                     # R12
            - fkinet(16);                    # R16
  # x(5): H2O
  xdot(5) = fkinet(1) - rkinet(1)            # R1
            + fkinet(6) - rkinet(6)          # R6
            + rkinet(13)                     # R13
            + fkinet(17)                     # R17
            + fkinet(18)                     # R18
            + fkinet(19) ;                   # R19
  # x(6): HBrO2
  xdot(6) = - fkinet(2) + rkinet(2)          # R2
            + fkinet(3) - rkinet(3)          # R3
            - fkinet(4) + rkinet(4)          # R5
            - fkinet(6) + rkinet(6)          # R6
            + fkinet(8)                      # R8
            + fkinet(17)                     # R17
            + fkinet(18) ;                   # R18
  # x(7): BrO3-
  xdot(7) = - fkinet(3) + rkinet(3)          # R3
            + fkinet(5)                      # R5
            - fkinet(6) - rkinet(6)          # R6
            - fkinet(17)                     # R17
            - fkinet(19) ;                   # R19
  # x(8): H2BrO2+
  xdot(8) = + fkinet(4) - rkinet(4)          # R4
            - fkinet(5) ;                    # R5
  # x(9): Br2O4
  xdot(9) = + fkinet(6) - rkinet(6)          # R6
            - fkinet(7) + rkinet(7) ;        # R7
  # x(10): BrO2*
  xdot(10) = 2 * finet(7) - rkinet(7)        # R7
             - fkinet(8)                     # R8
             - fkinet(9) ;                   # R9
  # x(11): H2Q
  xdot(11) = - fkinet(8)                     # R8
             + fkinet(10) - rkinet(10)       # R10
             + fkinet(15)                    # R15
             - fkinet(16)                    # R16
             - fkinet(17)                    # R17
             - fkinet(18)                    # R18
             + fkinet(19) ;                  # R19
  # x(12): HQ*
  xdot(12) = + fkinet(8)                     # R8
             - fkinet(10) + 2 * rkinet(10) ; # R10
  # x(13): Q
  xdot(13) = + fkinet(10) - fkinet(10)       # R10
             + fkinet(16)                    # R16
             + fkinet(17)                    # R17
             + fkinet(18) ;                  # R18
  # x(14): CHD
  xdot(14) = - fkinet(11) + rkinet(11)       # R11
             - fkinet(19) ;                  # R19
  # x(15): CHDE
  xdot(15) = + fkinet(11) - rkinet(11)       # R15
             - fkinet(12)                    # R12
             - fkinet(13) ;                  # R13
  # x(16): BrCHD
  xdot(16) = + fkinet(12)                    # R12
             + fkinet(13)                    # R13
             - fkinet(14);                   # R14
  # x(17): CHED
  xdot(17) = + fkinet(14)                    # R14
             - fkinet(15) ;                  # R15
             
endfunction

t = linspace (0, 0.01, 100);

if 1 > 2


y = lsode ("f", x0, t);

h1 = figure();
#plot (t, axz(t));
#subplot(2, 1, 1);
plot (t, y);

xlabel("time");
ylabel("concentration");

title(cstrcat(
			  "SK model ",
			  " f=", num2str(gf),
			  " [BrO_3^-]_0=", num2str(BrO30),
			  " [BrCHD]_0=", num2str(BrCHD0),
			  " k_{BrO3}=", num2str(kBrO3),
			  " k_{BrCHD}=", num2str(kBrCHD)
			  )
	  );
#legend("[HBrO_2]", "[Br^-]", "[H_2Q]");
print(h1, "sk.jpg", "-djpg");

endif



