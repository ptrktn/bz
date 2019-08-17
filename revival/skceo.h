#ifndef __ERHELPER_H__
#define __ERHELPER_H__

/*
 Br + HOBr + H <==> Br2 + H2O | 8E9 80                            (R1)
 Br + HBrO2 + H <==> HOBr + HOBr | 2.5E6 2E-5                     (R2)
 Br + BrO3 + H + H <==> HOBr + HBrO2 | 1.2 3.2                    (R3)
 HBrO2 + H <==> H2BrO2 | 2E6 1E8                                  (R4)
 HBrO2 + H2BrO2 ==> BrO3 + HOBr + H + H | 1.7E5                   (R5)
 HBrO2 + BrO3 + H <==> Br2O4 + H2O | 48 3.2E3                     (R6)
 Br2O4 <==> BrO2 + BrO2 | 7.5E4 1.4E9                             (R7)
 H2Q + BrO2 + BrO2 ==> HBrO2 + HBrO2 + Q | 2E6                    (R8)
 Red + BrO2 + H <==> Ox + HBrO2 | 1E7                             (R9)
 CHD + H <==> CHDE + H | 2.1E-4 5.2E2                             (R10)
 CHDE + Br2 ==> BrCHD + Br + H | 2.8E9                            (R11)
 BrCHD + H ==> CHED + Br + H + H | 5E-5                           (R12)
 CHED + H ==> H2Q + H | 1.9*1E-4                                  (R13)
 CHD + BrO3 + H ==> H2Q + HBrO2 + H2O | 2E-5                      (R14)
 CHD + HBrO2 ==> H2Q + HOBr + H2O | 5                             (R15)
 H2Q + BrO3 + H ==> Q + HBrO2 + H2O | 2E-2                        (R16)
 H2Q + HOBr ==> Q + Br + H + H2O | 6E5                            (R17)
 H2Q + Br2 ==> Q + Br + Br + H + H | 1E4                          (R18)
 Ox + Ox + CHD ==> Red + Red + H2Q + H + H | 0.11                 (R19)
 Ox + Ox + BrCHD ==> Q + Br + H + H + H + Red + Red | 0.051       (R20)
 Ox + Ox + H2Q ==> Red + Red + Q + H + H | 6E3                    (R21)
 Red + Red + BrO3 + H + H + H ==> Ox + Ox + HBrO2 + H2O | 0.22    (R22)

 x(1) HOBr
 x(2) Br2
 x(3) Br
 x(4) HBrO2
 x(5) BrO3
 x(6) H2BrO2
 x(7) Br2O4
 x(8) BrO2
 x(9) H2Q
 x(10) Ox
 x(11) Red
 x(12) CHD
 x(13) CHDE
 x(14) BrCHD
 x(15) CHED

 Plot command:

 xplot.sh erhelper.mat 'HOBr Br2 Br HBrO2 BrO3 H2BrO2 Br2O4 BrO2 H2Q Ox Red CHD CHDE BrCHD CHED'
*/

#define x(i) (x[i-1])
#define x0(i) (x[i])
#define xdot(i) (xdot[i-1])
#define kf(i) kf[i]
#define kr(i) kr[i]

/* Species in excess H H2O Q */
#define H (1.29)
#define H2O (1)
#define Q (1)

#define NEQ 15
#define N_REACTIONS 22
#define T_END 15000
#define T_DELTA (1/ (double) 33)
#define LSODE_ATOL 1E-6
#define LSODE_RTOL 1E-6
#define LSODE_HMAX 0.001

static double kf[N_REACTIONS+1], kr[N_REACTIONS+1];

static void erhelper_init(double *x, double *abstol, double *reltol)
{
	abstol[0] = LSODE_ATOL;
	reltol[0] = LSODE_RTOL;
	x0(0) = 0;
	abstol[1] = LSODE_ATOL;
	reltol[1] = LSODE_RTOL;
	x0(1) = 0;
	abstol[2] = LSODE_ATOL;
	reltol[2] = LSODE_RTOL;
	x0(2) = 0;
	abstol[3] = LSODE_ATOL;
	reltol[3] = LSODE_RTOL;
	x0(3) = 0;
	abstol[4] = LSODE_ATOL;
	reltol[4] = LSODE_RTOL;
	x0(4) = 0;
	abstol[5] = LSODE_ATOL;
	reltol[5] = LSODE_RTOL;
	x0(5) = 0;
	abstol[6] = LSODE_ATOL;
	reltol[6] = LSODE_RTOL;
	x0(6) = 0;
	abstol[7] = LSODE_ATOL;
	reltol[7] = LSODE_RTOL;
	x0(7) = 0;
	abstol[8] = LSODE_ATOL;
	reltol[8] = LSODE_RTOL;
	x0(8) = 0;
	abstol[9] = LSODE_ATOL;
	reltol[9] = LSODE_RTOL;
	x0(9) = 0;
	abstol[10] = LSODE_ATOL;
	reltol[10] = LSODE_RTOL;
	x0(10) = 0;
	abstol[11] = LSODE_ATOL;
	reltol[11] = LSODE_RTOL;
	x0(11) = 0;
	abstol[12] = LSODE_ATOL;
	reltol[12] = LSODE_RTOL;
	x0(12) = 0;
	abstol[13] = LSODE_ATOL;
	reltol[13] = LSODE_RTOL;
	x0(13) = 0;
	abstol[14] = LSODE_ATOL;
	reltol[14] = LSODE_RTOL;
	x0(14) = 0;
	abstol[15] = LSODE_ATOL;
	reltol[15] = LSODE_RTOL;
	x0(15) = 0;

	/* initial conditions */
	/* Br */
	x0(3) = 1E-5 ;
	/* BrO3 */
	x0(5) = 0.15 ;
	/* Red */
	x0(11) = 3E-4 ;
	/* CHD */
	x0(12) = 0.1 ;

	/* forward reaction rates */
	kf(1) = 8E9 ;
	kf(2) = 2.5E6 ;
	kf(3) = 1.2 ;
	kf(4) = 2E6 ;
	kf(5) = 1.7E5 ;
	kf(6) = 48 ;
	kf(7) = 7.5E4 ;
	kf(8) = 2E6 ;
	kf(9) = 1E7 ; /* [Fe(phen)3]2+ */
	kf(9) = 6.2E3;
	kf(10) = 2.1E-4 ;
	kf(11) = 2.8E9 ;
	kf(12) = 5E-5 ;
	kf(13) = 1.9*1E-4 ;
	kf(14) = 2E-5 ;
	kf(15) = 5 ;
	kf(16) = 2E-2 ;
	kf(17) = 6E5 ;
	kf(18) = 1E4 ;
	kf(19) = 0.11 ;
	kf(19) = 0.47 ;
	kf(20) = 0.051 ;
	kf(20) = 0.51 ;
	kf(21) = 1E2 ;
	kf(22) = 0.02 ;
	kf(22) = 0;

	/* reverse reaction rates */
	kr(1) = 80 ;
	kr(2) = 2E-5 ;
	kr(3) = 3.2 ;
	kr(4) = 1E8 ;
	kr(6) = 3.2E3 ;
	kr(7) = 1.4E9 ;
	kr(9) = 0 ; /* [Fe(phen)3]2+ */
	kr(9) = 1.2E4;
	kr(10) = 5.2E2 ;

}

/* 
   ATTENTION: There are exceptions in mass action kinetics. 
              Refer to the publication for details.
*/

static void fex(double t, double *x, double *xdot, void *data)
{
    /* HOBr */
    xdot(1) = - 1 *  kf(1)  *  x(3)  *  x(1)  *  H  + 1 *  kr(1)  *  x(2)  *  H2O  + 2 *  kf(2)  *  x(3)  *  x(4)  *  H  - 2 *  kr(2)  *  x(1)  *  x(1)  + 1 *  kf(3)  *  x(3)  *  x(5)  *  H  *  H  - 1 *  kr(3)  *  x(1)  *  x(4)  + 1 *  kf(5)  *  x(4)  *  x(6)  + 1 *  kf(15)  *  x(12)  *  x(4)  - 1 *  kf(17)  *  x(9)  *  x(1)  ; 
    /* Br2 */
    xdot(2) = + 1 *  kf(1)  *  x(3)  *  x(1)  *  H  - 1 *  kr(1)  *  x(2)  *  H2O  - 1 *  kf(11)  *  x(13)  *  x(2)  - 1 *  kf(18)  *  x(9)  *  x(2)  ; 
    /* Br */
    xdot(3) = - 1 *  kf(1)  *  x(3)  *  x(1)  *  H  + 1 *  kr(1)  *  x(2)  *  H2O  - 1 *  kf(2)  *  x(3)  *  x(4)  *  H  + 1 *  kr(2)  *  x(1)  *  x(1)  - 1 *  kf(3)  *  x(3)  *  x(5)  *  H  *  H  + 1 *  kr(3)  *  x(1)  *  x(4)  + 1 *  kf(11)  *  x(13)  *  x(2)  + 1 *  kf(12)  *  x(14)  *  H  + 1 *  kf(17)  *  x(9)  *  x(1)  + 2 *  kf(18)  *  x(9)  *  x(2)  + 1 *  kf(20)  *  x(10)  *  x(14)  ; 
    /* HBrO2 */
    xdot(4) = - 1 *  kf(2)  *  x(3)  *  x(4)  *  H  + 1 *  kr(2)  *  x(1)  *  x(1)  + 1 *  kf(3)  *  x(3)  *  x(5)  *  H  *  H  - 1 *  kr(3)  *  x(1)  *  x(4)  - 1 *  kf(4)  *  x(4)  *  H  + 1 *  kr(4)  *  x(6)  - 1 *  kf(5)  *  x(4)  *  x(6)  - 1 *  kf(6)  *  x(4)  *  x(5)  *  H  + 1 *  kr(6)  *  x(7)  *  H2O  + 2 *  kf(8)  *  x(9)  *  x(8)  + 1 *  kf(9)  *  x(11)  *  x(8)  *  H  - 1 *  kr(9)  *  x(10)  *  x(4)  + 1 *  kf(14)  *  x(12)  *  x(5)  *  H  - 1 *  kf(15)  *  x(12)  *  x(4)  + 1 *  kf(16)  *  x(9)  *  x(5)  *  H  + 1 *  kf(22)  *  x(11)  *  x(5)  *  H  *  H  ; 
    /* BrO3 */
    xdot(5) = - 1 *  kf(3)  *  x(3)  *  x(5)  *  H  *  H  + 1 *  kr(3)  *  x(1)  *  x(4)  + 1 *  kf(5)  *  x(4)  *  x(6)  - 1 *  kf(6)  *  x(4)  *  x(5)  *  H  + 1 *  kr(6)  *  x(7)  *  H2O  - 1 *  kf(14)  *  x(12)  *  x(5)  *  H  - 1 *  kf(16)  *  x(9)  *  x(5)  *  H  - 1 *  kf(22)  *  x(11)  *  x(5)  *  H  *  H  ; 
    /* H2BrO2 */
    xdot(6) = + 1 *  kf(4)  *  x(4)  *  H  - 1 *  kr(4)  *  x(6)  - 1 *  kf(5)  *  x(4)  *  x(6)  ; 
    /* Br2O4 */
    xdot(7) = + 1 *  kf(6)  *  x(4)  *  x(5)  *  H  - 1 *  kr(6)  *  x(7)  *  H2O  - 1 *  kf(7)  *  x(7)  + 1 *  kr(7)  *  x(8)  *  x(8)  ; 
    /* BrO2 */
	xdot(8) = + 2 *  kf(7)  *  x(7)  - 2 *  kr(7)  *  x(8)  *  x(8)  - 2 *  kf(8)  *  x(9)  *  x(8)  - 1 *  kf(9)  *  x(11)  *  x(8)  *  H  + 1 *  kr(9)  *  x(10)  *  x(4)  ; 
    /* H2Q */
    xdot(9) = - 1 *  kf(8)  *  x(9)  *  x(8)  + 1 *  kf(13)  *  x(15)  *  H  + 1 *  kf(14)  *  x(12)  *  x(5)  *  H  + 1 *  kf(15)  *  x(12)  *  x(4)  - 1 *  kf(16)  *  x(9)  *  x(5)  *  H  - 1 *  kf(17)  *  x(9)  *  x(1)  - 1 *  kf(18)  *  x(9)  *  x(2)  + 1 *  kf(19)  *  x(10)  *  x(12)  - 1 *  kf(21)  *  x(10)  *  x(9)  ; 
    /* Ox */
    xdot(10) = + 1 *  kf(9)  *  x(11)  *  x(8)  *  H  - 1 *  kr(9)  *  x(10)  *  x(4)  - 2 *  kf(19)  *  x(10)  *  x(12)  - 2 *  kf(20)  *  x(10)  *  x(14)  - 2 *  kf(21)  *  x(10)  *  x(9)  + 2 *  kf(22)  *  x(11)  *  x(5)  *  H  *  H  ; 
    /* Red */
    xdot(11) = - 1 *  kf(9)  *  x(11)  *  x(8)  *  H  + 1 *  kr(9)  *  x(10)  *  x(4)  + 2 *  kf(19)  *  x(10)  *  x(12)  + 2 *  kf(20)  *  x(10)  *  x(14)  + 2 *  kf(21)  *  x(10)  *  x(9)  - 2 *  kf(22)  *  x(11)  *  x(5)  *  H  *  H  ; 
    /* CHD */
    xdot(12) = - 1 *  kf(10)  *  x(12)  *  H  + 1 *  kr(10)  *  x(13)  *  H  - 1 *  kf(14)  *  x(12)  *  x(5)  *  H  - 1 *  kf(15)  *  x(12)  *  x(4)  - 1 *  kf(19)  *   x(10)  *  x(12)  ; 
    /* CHDE */
    xdot(13) = + 1 *  kf(10)  *  x(12)  *  H  - 1 *  kr(10)  *  x(13)  *  H  - 1 *  kf(11)  *  x(13)  *  x(2)  ; 
    /* BrCHD */
    xdot(14) = + 1 *  kf(11)  *  x(13)  *  x(2)  - 1 *  kf(12)  *  x(14)  *  H  - 1 *  kf(20)  *  x(10)  *  x(14)  ; 
    /* CHED */
    xdot(15) = + 1 *  kf(12)  *  x(14)  *  H  - 1 *  kf(13)  *  x(15)  *  H  ; 

}

#endif
