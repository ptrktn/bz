/* ------------------------------------------------------------------------- 
 * ezstep3d.c -- Time Stepping Routines 
 *
 * Copyright (C) 1996, 1997, 1998 Dwight Barkley and Matthew Dowle
 *
 * RCS Information
 * ---------------------------
 * $Revision: 1.1.1.1 $
 * $Date: 2003/07/05 15:08:18 $
 * ------------------------------------------------------------------------- */

#include "ezscroll.h"
#include "ezstep3d.h"

/* -------------------------------------------------------------------------
 * I use many pre-processor macros to make the time-stepping flexible.
 * See ezstep3d.h for the definitions.
 * ------------------------------------------------------------------------- */

static void  Average_periodic_directions (void);
static void  Impose_boundary_conditions (Real *w, Real *sigma_w);
static int   s1, s2;

const Real zero       = 0.;
const Real half       = 0.5;
const Real third      = 1./3.;
const Real one        = 1.;
const Real two        = 2.;
const Real six        = 6.;
const Real twentyfour = 24.;

/* ========================================================================= */

void Step (void)
{
  Real     u_thresh;
  register int i, j, k;
  register int index, index_1, index_2;
  
  /* Interchange s1 and s2 */
  s1 = 1-s1;
  s2 = 1-s2;
  
  Average_periodic_directions();

  /* ------------------------------------- 
   * Main Loop (almost all work done here) 
   * ------------------------------------- */

  for(k=1;k<=NZ;k++) {
    for(j=1;j<=NY;j++) {
      index = INDEX(0,j,k);
      index_1 = index + s1*field_size;
      index_2 = index + s2*field_size;
      for(i=1;i<=NX;i++) {
	index++; index_1++; index_2++;

#if SPLIT        /* (split=1 is best for large time steps, else use split=0) */
	u[index] += U_DIFFUSION(index_1);
	v[index] += V_DIFFUSION(index_1);
	ZERO_USED_SUMS(index_1);
#endif

	if(u[index]<delta) { 
	  u[index] = zero;
	  v[index] = V_KINETICS(zero,v[index]);
#if !SPLIT
	  u[index] += U_DIFFUSION(index_1);
	  v[index] += V_DIFFUSION(index_1);
#endif
	}
	else { 
	  u_thresh = U_THRESHOLD(v[index]);
	  v[index] = V_KINETICS(u[index],v[index]);
	  u[index] = U_KINETICS(u[index],u_thresh);
#if !SPLIT
	  u[index] += U_DIFFUSION(index_1);
	  v[index] += V_DIFFUSION(index_1);
#endif
	  ADD_TO_U_SUM(index,index_2);
	}
	ADD_TO_V_SUM(index,index_2);
	ZERO_USED_SUMS(index_1);
      }
    }
  }
  Impose_boundary_conditions(u, sigma_u);
#if V_DIFF_ON        
  Impose_boundary_conditions(v, sigma_v);
#endif
}    
/* ========================================================================= */

void Step_ini (void)
{
  int i, j, k;
  int index, index_1, index_2;

  /* Set initial s1 and s2 */
  s1 = 0;
  s2 = 1;

  Average_periodic_directions();

  /* Initialize spatial sums. */

  for(k=1;k<=NZ;k++) {
    for(j=1;j<=NY;j++) {
      for(i=1;i<=NX;i++) {
	index = INDEX(i,j,k);
	index_1 = index + s1*field_size;
	index_2 = index + s2*field_size;
	ADD_TO_U_SUM(index,index_2);
	ADD_TO_V_SUM(index,index_2);
	ZERO_USED_SUMS(index_1);
      }
    }
  }
  Impose_boundary_conditions(u, sigma_u);
#if V_DIFF_ON        
  Impose_boundary_conditions(v, sigma_v);
#endif
}
/* ========================================================================= */

void Average_periodic_directions (void)
{
  /* ----------------------------------------------------------------------- 
   * In my implementation, if periodic boundary conditions are imposed in a
   * direction (x-direction, say), then the fields at grid points 1 and NX
   * should be the same.  This routine insures this by replacing these values
   * by their average.  This should be called by step_ini() to insure these
   * values are initially the same. They should thereafter remain the same
   * except for the effects of roundoff.  I have put a call to this routine
   * in step() and have put a counter here so that the averaging is done only
   * after a large number of time steps (500).  In my tests the fields differ
   * after this number of time steps by only a few times machine epsilon.
   * Note: if the averaging is done every time step then a numerical
   * instability occurs. 
   * ----------------------------------------------------------------------- */

  static int count=0;

  if(count++%500) return;

#if PBC_x
  {
    int j, k;
    for(k=1;k<=NZ;k++) {
      for(j=1;j<=NY;j++) {
	U(NX,j,k) = U(1,j,k) = half*(U(NX,j,k)+U(1,j,k));
	V(NX,j,k) = V(1,j,k) = half*(V(NX,j,k)+V(1,j,k));
      }
    }
  }
#endif
#if PBC_y
  {
    int i, k;
    for(k=1;k<=NZ;k++) {
      for(i=1;i<=NX;i++) {
	U(i,NY,k) = U(i,1,k) = half*(U(i,NY,k)+U(i,1,k));
	V(i,NY,k) = V(i,1,k) = half*(V(i,NY,k)+V(i,1,k));
      }
    }
  }
#endif
#if PBC_z
  {
    int i, j;
    for(j=1;j<=NY;j++) {
      for(i=1;i<=NX;i++) {
	U(i,j,NZ) = U(i,j,1) = half*(U(i,j,NZ)+U(i,j,1));
	V(i,j,NZ) = V(i,j,1) = half*(V(i,j,NZ)+V(i,j,1));
      }
    }
  }
#endif
}
/* ========================================================================= */

/* Define W and Sigma_w macros.  w is generic field (u or v) */
#define W(i,j,k)          w[INDEX(i,j,k)]
#define Sigma_w(s,i,j,k)  sigma_w[(s)*FIELD_SIZE + INDEX(i,j,k)]

static void Impose_boundary_conditions (Real *w, Real *sigma_w)
{
  int i,j,k;

  /* First the values of w on the "real" part of the mesh are copied to
   * "fictitious" boundary points.  The way points are copied depend on
   * whether Neumann or periodic boundary conditions are being imposed.
   *
   * Note that the indices i, j, k do not range over the same values in each
   * case (x, y, z).  This is correct though a bit subtle. */

  /* Set fictitious points in x-direction */
  for(k=1;k<=NZ;k++) { 
    for(j=1;j<=NY;j++) { 
#if PBC_x
      W(0   ,j,k) = W(NX-1,j,k);  
      W(NX+1,j,k) = W(2   ,j,k);  
#else
      W(0   ,j,k) = W(2   ,j,k);  
      W(NX+1,j,k) = W(NX-1,j,k);  
#endif
    }
  }

  /* Set fictitious points in y-direction */
  for(k=1;k<=NZ;k++) { 
    for(i=0;i<=NX+1;i++) { 
#if PBC_y
      W(i,0   ,k) = W(i,NY-1,k);  
      W(i,NY+1,k) = W(i,2   ,k);  
#else
      W(i,0   ,k) = W(i,2   ,k);  
      W(i,NY+1,k) = W(i,NY-1,k);  
#endif
    }
  }

  /* Set fictitious points in z-direction */
  for(j=0;j<=NY+1;j++) { 
    for(i=0;i<=NX+1;i++) { 
#if PBC_z
      W(i,j,0   ) = W(i,j,NZ-1);  
      W(i,j,NZ+1) = W(i,j,2   );  
#else
      W(i,j,0   ) = W(i,j,2   );  
      W(i,j,NZ+1) = W(i,j,NZ-1);  
#endif
    }
  }

#if NINETEENPT

/* ---------------------------------
 * Nineteen-point Laplacian formulas
 * --------------------------------- */

  for(j=1;j<=NY;j++) {
    for(i=1;i<=NX;i++) {
      Sigma_w(s2,i  ,j  ,1) += two*W(i,j,0);  
      Sigma_w(s2,i+1,j  ,1) +=     W(i,j,0);  
      Sigma_w(s2,i-1,j  ,1) +=     W(i,j,0); 
      Sigma_w(s2,i  ,j+1,1) +=     W(i,j,0);  
      Sigma_w(s2,i  ,j-1,1) +=     W(i,j,0); 

      Sigma_w(s2,i  ,j  ,NZ) += two*W(i,j,NZ+1); 
      Sigma_w(s2,i+1,j  ,NZ) +=     W(i,j,NZ+1);  
      Sigma_w(s2,i-1,j  ,NZ) +=     W(i,j,NZ+1); 
      Sigma_w(s2,i  ,j+1,NZ) +=     W(i,j,NZ+1);  
      Sigma_w(s2,i  ,j-1,NZ) +=     W(i,j,NZ+1); 
    }
  }
 
  for(k=1;k<=NZ;k++) {
    for(i=1;i<=NX;i++) {
      Sigma_w(s2,i  ,1,k  ) += two*W(i,0,k);  
      Sigma_w(s2,i+1,1,k  ) +=     W(i,0,k);  
      Sigma_w(s2,i-1,1,k  ) +=     W(i,0,k); 
      Sigma_w(s2,i  ,1,k+1) +=     W(i,0,k);  
      Sigma_w(s2,i  ,1,k-1) +=     W(i,0,k); 

      Sigma_w(s2,i  ,NY,k  ) += two*W(i,NY+1,k); 
      Sigma_w(s2,i+1,NY,k  ) +=     W(i,NY+1,k);  
      Sigma_w(s2,i-1,NY,k  ) +=     W(i,NY+1,k); 
      Sigma_w(s2,i  ,NY,k+1) +=     W(i,NY+1,k);  
      Sigma_w(s2,i  ,NY,k-1) +=     W(i,NY+1,k); 
    }
  } 

  for(k=1;k<=NZ;k++) {
    for(j=1;j<=NY;j++) {
      Sigma_w(s2,1,j  ,k  ) += two*W(0,j,k); 
      Sigma_w(s2,1,j+1,k  ) +=     W(0,j,k);  
      Sigma_w(s2,1,j-1,k  ) +=     W(0,j,k); 
      Sigma_w(s2,1,j  ,k+1) +=     W(0,j,k);  
      Sigma_w(s2,1,j  ,k-1) +=     W(0,j,k); 

      Sigma_w(s2,NX,j  ,k  ) += two*W(NX+1,j,k); 
      Sigma_w(s2,NX,j+1,k  ) +=     W(NX+1,j,k);  
      Sigma_w(s2,NX,j-1,k  ) +=     W(NX+1,j,k); 
      Sigma_w(s2,NX,j  ,k+1) +=     W(NX+1,j,k);  
      Sigma_w(s2,NX,j  ,k-1) +=     W(NX+1,j,k); 
    }
  }

  for(i=1;i<=NX;i++) {
    Sigma_w(s2,i,1 ,1 ) += W(i,  0 ,  0 );  
    Sigma_w(s2,i,NY,1 ) += W(i,NY+1,  0 );  
    Sigma_w(s2,i,1 ,NZ) += W(i,  0 ,NZ+1);  
    Sigma_w(s2,i,NY,NZ) += W(i,NY+1,NZ+1);  
  }

  for(j=1;j<=NY;j++) {
    Sigma_w(s2,1 ,j,1 ) += W(  0 ,j,  0 );  
    Sigma_w(s2,NX,j,1 ) += W(NX+1,j,  0 );  
    Sigma_w(s2,1 ,j,NZ) += W(  0 ,j,NZ+1);  
    Sigma_w(s2,NX,j,NZ) += W(NX+1,j,NZ+1);  
  }

  for(k=1;k<=NZ;k++) {
    Sigma_w(s2,1 ,1 ,k) += W(  0 ,  0 ,k);  
    Sigma_w(s2,NX,1 ,k) += W(NX+1,  0 ,k);  
    Sigma_w(s2,1 ,NY,k) += W(  0 ,NY+1,k);  
    Sigma_w(s2,NX,NY,k) += W(NX+1,NY+1,k);  
  }

#else

/* ------------------------------
 * Seven-point Laplacian formulas 
 * ------------------------------ */

  for(j=1;j<=NY;j++) {
    for(i=1;i<=NX;i++) {
      Sigma_w(s2,i,j,1 ) += W(i,j,0   );  
      Sigma_w(s2,i,j,NZ) += W(i,j,NZ+1); 
    }
  }

  for(k=1;k<=NZ;k++) {
    for(i=1;i<=NX;i++) {
      Sigma_w(s2,i,1 ,k) += W(i,0   ,k);  
      Sigma_w(s2,i,NY,k) += W(i,NY+1,k); 
    }
  }

  for(k=1;k<=NZ;k++) {
    for(j=1;j<=NY;j++) {
      Sigma_w(s2,1 ,j,k) += W(0   ,j,k); 
      Sigma_w(s2,NX,j,k) += W(NX+1,j,k); 
    }
  }

#endif  /* end if Nineteen-point Laplacian formulas */
}
/* ========================================================================= */

#undef W
#undef Sigma_w



