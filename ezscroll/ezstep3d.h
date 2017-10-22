/* ------------------------------------------------------------------------- 
 * ezstep3d.h -- Macros for EZSTEP
 *
 * Copyright (C) 1996, 1997, 1998 Dwight Barkley and Matthew Dowle
 *
 * RCS Information
 * ---------------------------
 * $Revision: 1.1.1.1 $
 * $Date: 2003/07/05 15:08:18 $
 * ------------------------------------------------------------------------- */

#include "ezscroll.h"
#ifndef _EZSTEP3D_
#define _EZSTEP3D_

/* ---------------------------------------------------------------------
 * Define the macros U_THRESHOLD(v) and G(u,v) according to the model
 * you wish to simulate. In principle these can be almost anything you
 * want.  Three examples are included.  
 * --------------------------------------------------------------------- */

#if 1  /* Standard Model */
  #define U_THRESHOLD(v)  ( one_o_a*(v) + b_o_a )
  #define G(u,v)          ( (u)-(v) )
#endif

#if 0  /* Simple Model for Turbulent Spirals */
  #define U_THRESHOLD(v)  ( one_o_a*(v) + b_o_a )
  #define G(u,v)          ( (u)*(u)*(u) - (v) )
#endif

#if 0  /* Bar and Eiswirth Model (Phys. Rev. E V. 48, p. 1635, 1993).
	* I do not include their case u>1 because it is not relevant. */
  #define U_THRESHOLD(v)  ( one_o_a*(v) + b_o_a )
  #define G(u,v)          (-(v) + ( ((u)<third) ? 0 : \
                                    (one-6.75*(u)*(1-(u))*(1-(u))) ) )  
#endif

/* ------------------------------------------------------------------------- 
 *          In general you should not need to change anything below          
 * ------------------------------------------------------------------------- */

/* I rely on the compiler to take care of most optimization.  The only place
 * where I give the compiler help is with the Laplacian formulas because
 * none of the compiles do the obvious thing.  */

/* ------------------------------------- 
 * Defining the kinetics macros:         
 * U_KINETICS(u,uth) and V_KINETICS(u,v) 
 * ------------------------------------- */

#if EXPLICIT   
     /* Explicit u-kinetics */

  #define U_KINETICS(u,uth) ( (u)+dt_o_eps*(u)*(one-(u))*((u)-(uth)) )

#else          
     /* Implicit u-kinetics */
     /* The original (Physica 49D) scheme can be obtained by defining
      * F2m and F2p to be one */

  #define F1m(u,uth) ( dt_o_eps*(one-(u))*((u)-(uth)) )
  #define F1p(u,uth) ( dt_o_eps*(u)*((u)-(uth)) )
  #define F2m(u,uth) ( one + dt_o_2eps*((uth)-(u)*(u)) )
  #define F2p(u,uth) ( one + dt_o_2eps*(two*(u) -(uth)-(u)*(u)) )

  #define U_KINETICS(u,uth) (                                      \
          ( (u) < (uth) ) ?                                        \
          (u)/(one-F1m(u,uth)*F2m(u,uth) ) :                       \
          ((u)+F1p(u,uth)*F2p(u,uth))/(one+F1p(u,uth)*F2p(u,uth)) )

#endif

#define V_KINETICS(u,v) ( (v)+dt*G(u,v) )

/* ---------------------------------------- 
 * Defining the diffusion macros:           
 * U_DIFFUSION, V_DIFFUSION, ZERO_USED_SUMS 
 * ---------------------------------------- */

#if V_DIFF_ON        
     /* v is diffusing */
  #define U_DIFFUSION(index_1)     (dt_o_wh2   * sigma_u[index_1]) 
  #define V_DIFFUSION(index_1)     (dtDv_o_wh2 * sigma_v[index_1]) 
  #define ZERO_USED_SUMS(index_1)  sigma_u[index_1] = sigma_v[index_1] = zero;
#else
     /* v is not diffusing */
  #define U_DIFFUSION(index_1)     (dt_o_wh2 * sigma_u[index_1]) 
  #define V_DIFFUSION(index_1)     zero
  #define ZERO_USED_SUMS(index_1)  sigma_u[index_1] = zero;
#endif

/* -------------------------------- 
 * Defining the spatial-sum macros: 
 * ADD_TO_U_SUM, ADD_TO_V_SUM     
 * -------------------------------- */

#if NINETEENPT
/* 19-point Laplacian formula */
  #define ADD_TO_U_SUM(index,index_2) \
   {Real stupid_cc = u[index]; \
     sigma_u[(index_2)]  -= twentyfour*stupid_cc; \
     sigma_u[(index_2)+K_INC]   += two*stupid_cc; \
     sigma_u[(index_2)-K_INC]   += two*stupid_cc; \
     sigma_u[(index_2)+J_INC]   += two*stupid_cc; \
     sigma_u[(index_2)-J_INC]   += two*stupid_cc; \
     sigma_u[(index_2)+I_INC]   += two*stupid_cc; \
     sigma_u[(index_2)-I_INC]   += two*stupid_cc; \
     sigma_u[(index_2)+J_INC+K_INC] += stupid_cc; \
     sigma_u[(index_2)+J_INC-K_INC] += stupid_cc; \
     sigma_u[(index_2)-J_INC+K_INC] += stupid_cc; \
     sigma_u[(index_2)-J_INC-K_INC] += stupid_cc; \
     sigma_u[(index_2)+I_INC+K_INC] += stupid_cc; \
     sigma_u[(index_2)+I_INC-K_INC] += stupid_cc; \
     sigma_u[(index_2)-I_INC+K_INC] += stupid_cc; \
     sigma_u[(index_2)-I_INC-K_INC] += stupid_cc; \
     sigma_u[(index_2)+I_INC+J_INC] += stupid_cc; \
     sigma_u[(index_2)+I_INC-J_INC] += stupid_cc; \
     sigma_u[(index_2)-I_INC+J_INC] += stupid_cc; \
     sigma_u[(index_2)-I_INC-J_INC] += stupid_cc; \
   }
#else
/* 7-point Laplacian formula */
  #define ADD_TO_U_SUM(index,index_2) \
   {Real stupid_cc = u[index]; \
      sigma_u[(index_2)]   -= six*stupid_cc; \
      sigma_u[(index_2)+K_INC] += stupid_cc; \
      sigma_u[(index_2)-K_INC] += stupid_cc; \
      sigma_u[(index_2)+J_INC] += stupid_cc; \
      sigma_u[(index_2)-J_INC] += stupid_cc; \
      sigma_u[(index_2)+I_INC] += stupid_cc; \
      sigma_u[(index_2)-I_INC] += stupid_cc; \
   }
#endif         

#if !V_DIFF_ON   
/* v is not diffusing */
  #define ADD_TO_V_SUM(index,index_2) 
#else            
/* v is diffusing */
#if NINETEENPT   
/* 19-point Laplacian formula */
  #define ADD_TO_V_SUM(index,index_2) \
   {Real stupid_cc = v[index];  \
     sigma_v[(index_2)]  -= twentyfour*stupid_cc; \
     sigma_v[(index_2)+K_INC]   += two*stupid_cc; \
     sigma_v[(index_2)-K_INC]   += two*stupid_cc; \
     sigma_v[(index_2)+J_INC]   += two*stupid_cc; \
     sigma_v[(index_2)-J_INC]   += two*stupid_cc; \
     sigma_v[(index_2)+I_INC]   += two*stupid_cc; \
     sigma_v[(index_2)-I_INC]   += two*stupid_cc; \
     sigma_v[(index_2)+J_INC+K_INC] += stupid_cc; \
     sigma_v[(index_2)+J_INC-K_INC] += stupid_cc; \
     sigma_v[(index_2)-J_INC+K_INC] += stupid_cc; \
     sigma_v[(index_2)-J_INC-K_INC] += stupid_cc; \
     sigma_v[(index_2)+I_INC+K_INC] += stupid_cc; \
     sigma_v[(index_2)+I_INC-K_INC] += stupid_cc; \
     sigma_v[(index_2)-I_INC+K_INC] += stupid_cc; \
     sigma_v[(index_2)-I_INC-K_INC] += stupid_cc; \
     sigma_v[(index_2)+I_INC+J_INC] += stupid_cc; \
     sigma_v[(index_2)+I_INC-J_INC] += stupid_cc; \
     sigma_v[(index_2)-I_INC+J_INC] += stupid_cc; \
     sigma_v[(index_2)-I_INC-J_INC] += stupid_cc; \
  }
#else          
/* 7-point Laplacian formula */
  #define ADD_TO_V_SUM(index,index_2) \
   {Real stupid_cc = v[index]; \
      sigma_v[(index_2)]   -= six*stupid_cc; \
      sigma_v[(index_2)+K_INC] += stupid_cc; \
      sigma_v[(index_2)-K_INC] += stupid_cc; \
      sigma_v[(index_2)+J_INC] += stupid_cc; \
      sigma_v[(index_2)-J_INC] += stupid_cc; \
      sigma_v[(index_2)+I_INC] += stupid_cc; \
      sigma_v[(index_2)-I_INC] += stupid_cc; \
  }
#endif 
#endif
#endif /*  _EZSTEP3D_  */
