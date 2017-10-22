/* ------------------------------------------------------------------------- *
 * ezscroll.h -- header file for EZ-Scroll
 *
 * Copyright (C) 1996, 1997, 1998 Dwight Barkley and Matthew Dowle
 *
 * RCS Information
 * ---------------------------
 * $Revision: 1.1.1.1 $
 * $Date: 2003/07/05 15:08:18 $
 * ------------------------------------------------------------------------- */

#ifndef _EZSCROLL_
#define _EZSCROLL_

/* ------------------------------------------------------------------------- */
/* These are the main parameters affecting the compilation of EZ-Scroll.     */
/* Define them according to your needs.                                      */
/*                                                                           */
typedef float  Real;      /* precision of Real variables (float or double)   */
                          /*                                                 */
#define V_DIFF_ON  0      /* if 1 then v-field diffuses                      */
#define EXPLICIT   0      /* if 1 then explicit u-kinetics, else implicit    */
#define SPLIT      1      /* if 1 then split diffusion and kinetics steps    */
#define NINETEENPT 1      /* if 1 then 19 pt Laplacian formulas, else 7 pt   */
#define PBC_x      0      /* if 1 then periodic BCs in x, else Neumann BCs   */
#define PBC_y      0      /* if 1 then periodic BCs in y, else Neumann BCs   */
#define PBC_z      0      /* if 1 then periodic BCs in z, else Neumann BCs   */
#define GRAPHICS   1      /* if 1 then run with interactive graphics         */
/* ------------------------------------------------------------------------- */

/* 
 * I always define min, max, and make sure M_PI is defined 
 * ------------------------------------------------------- */
#define min(a,b)      ((a)>(b) ? (b) : (a))   
#define max(a,b)      ((a)>(b) ? (a) : (b))   
#ifndef M_PI
#define M_PI	      3.14159265358979323846
#endif

/* --------------------------------------------------- 
 * Global variables used throughout the EZ-Scroll Code 
 * (I use lots of them)
 * --------------------------------------------------- */

extern Real *fields, *u, *v,             /* arrays for concentration fields */
            *sigma_u, *sigma_v;          /* and spatial sums (Laplacians) */
extern Real a_param, b_param, one_o_eps, /* model parameters */
            delta, grid_h, dt,           /* numerical parameters */
            dt_o_eps, dt_o_2eps,         /* useful parameter combinations */
            one_o_a, b_o_a,              /* useful parameter combinations */
            dt_o_wh2, dtDv_o_wh2;        /* useful parameter combinations */
extern int  nx, ny, nz,                  /* # of grid points per direction */
            field_size,                  /* array size for each field */
            nsteps,                      /* # time steps to take */
            istep,                       /* current time step */
            write_filament,              /* write filament flag */
            simulating_resolution,       /* graphics parameter */
            rotating_resolution,         /* graphics parameter */
            verbose,                     /* verbosity level */
            plot_step,                   /* plotting interval */
            hist_step;                   /* plotting interval */

/* -------------------------------------------------------------------------
 * All index ranges throughout the code are expressed in terms of NX, NY,
 * and NZ.  The code is generally more efficient if these are known numbers
 * at compile time, but then one must recompile for each change.  If the
 * values are known (eg specified on the compile line) then do nothing
 * here, otherwise define NX etc to be the *variables* nx, etc.
 * ------------------------------------------------------------------------- */

#ifndef NX
  #define NX  nx
#endif

#ifndef NY
  #define NY  ny
#endif

#ifndef NZ
  #define NZ  nz
#endif

/* ------------------------------------------------------------------------- 
 * Memory for the chemical fields (u and v) and the spatial sums (sigma_u    
 * and sigma_v) is allocated in Allocate_memory() in ezscroll.c.  These are      
 * allocated as long (single dimensional) arrays.  Here macros are defined   
 * so that one can easily reference a value corresponding to a particular    
 * grid point, i.e. macros are defined to treat all arrays as                
 * multi-dimensional. If you don't like the way I do this, you should be     
 * able to change it easily by making modifications here and in              
 * AllocateMem().  Let me know if you find a significant improvement.        
 *                                                                           
 * INDEX(i,j,k) converts grid point (i,j,k) to the array index.              
 *                                                                           
 * U(i,j,k)          --  u field at grid point (i,j,k)                       
 * V(i,j,k)          --  v field at grid point (i,j,k)                       
 * Sigma_u(s,i,j,k)  --  Spatial-sum array for u: s=0 or 1 (see ezstep3d.c)    
 * Sigma_v(s,i,j,k)  --  Spatial-sum array for v: s=0 or 1 (see ezstep.c)    
 * Fields (f,i,j,k)  --  array of fields: f=0 for u or f=1 for v.            
 *                       (it is necessary to have one array containing both  
 *                        u and v for the graphics routines in ezmarching)   
 * ------------------------------------------------------------------------- */

#define K_INC      ((NX+2)*(NY+2))  
#define J_INC       (NX+2)
#define I_INC        1
#define FIELD_SIZE ((NZ+2)*(NY+2)*(NX+2)) 

#define INDEX(i,j,k)  ((i)*I_INC + (j)*J_INC + (k)*K_INC)

#define U(i,j,k)          u[INDEX(i,j,k)]
#define V(i,j,k)          v[INDEX(i,j,k)]
#define Sigma_u(s,i,j,k)  sigma_u[(s)*FIELD_SIZE + INDEX(i,j,k)]
#define Sigma_v(s,i,j,k)  sigma_v[(s)*FIELD_SIZE + INDEX(i,j,k)]
#define Fields(f,i,j,k)   fields [(f)*FIELD_SIZE + INDEX(i,j,k)]


/* ------------------------------------------- 
 * Prototypes for public functions defined in: 
 * ------------------------------------------- */

/* ezscroll.c 
 * ---------- */
void Write_filament_data (float x0, float y0, float z0, 
			  float x1, float y1, float z1); 
void  Finish_up       (void);
void  Write_history   (int wrt_step);

/* ezstep3d.c 
 * ---------- */
void  Step      (void);
void  Step_ini  (void);

/* ezgraph3d.c 
 * ----------- */
void  Draw         (void);
void  Draw_ini     (int argc, char **argv, int initial_field);
int   Event_check  (void);
void  QuitX        (void);

/* ezmarching.c 
 * ------------ */
void  Marching_cubes  (unsigned int resolution, int field, int show_filament, 
		       int clipping, Real *plot_length);
void  Marching_ini    (void);

#endif /*  _EZSCROLL_  */


