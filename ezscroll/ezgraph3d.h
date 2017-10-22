/* ------------------------------------------------------------------------- */
/* ezgraph3d.h -- Macros for EZGRAPH3D
 *
 * Copyright (C) 1996, 1997, 1998 Dwight Barkley and Matthew Dowle
 *
 * RCS Information
 * ---------------------------
 * $Revision: 1.1.1.1 $
 * $Date: 2003/07/05 15:08:18 $
 * ------------------------------------------------------------------------- */

#ifndef _EZGRAPH3D_
#define _EZGRAPH3D_

#define WINDOW_TITLE "EZ-Scroll"

#define WINX         0      /* Window location, in pixels, from screen left */
#define WINY         0      /* Window location, in pixels, from screen top. */
#define WM_CTRLS_POS 0      /* If WM_CTRL_POS is 0 then the window is placed
			     * at (WINX, WINY). If WM_CTRL_POS is 1 then WINX
			     * and WINY are ignored and the location is
			     * determined by the window manager. */
#define WINSIZE      500    /* Window is square of this size in pixels. */
#define PLOT_SIZE    1.0    /* This controls the size of the simulation
			     * volume in the view port: >1.0 for larger
			     * size, <1.0 for smaller size. */

#define PERSPECTIVE  1      /* 1 for perspective projection, 0 for orthographic.
			     * see myReshape(). */

#define DISTANCE     5.0    /* This is the distance that the screen is from
			     * from the eye (measured in units of the largest
			     * side of the simulations volume as it appears
			     * on the screen). This is only relevant for
			     * perspective projection. see myReshape() and
			     * polarView(). */

#define BACKGROUND   0.0    /* Background color (R=G=B=BACKGROUND, so 0.0 gives
			       BLACK, 1.0 gives WHITE) */

#define START_PAUSED 1      /* If 1 then window is opened in paused mode
			     * showing initial condition. */

#define INITIAL_THETA 130.0 /* Initial view (any value in [-180,180)). */
#define INITIAL_PHI   45.0  /* Initial view (any value in [-180,180)). */
#define DELTA_VIEW_DEG 2.5  /* Increments of theta and phi when rotating 
                             * using vi "arrow" keys (-D_GLONLY) */
#define ROTATE_SCALE   0.5  /* Number of degrees of rotation corresponding to
			     * one pixel of mouse motion. */

/* --------------------------------------------- 
 * You probably should not change anything below 
 * --------------------------------------------- */

#define TRUE             1   
#define FALSE            0

#define U_FIELD          0     /* These are used to determine which field */
#define V_FIELD          1     /* (if any) is being plotted */
#define NO_FIELD        -1

#define ISO_LIST         1

#define MODE_SIMULATING  1   
#define MODE_VIEWING     2   
#define MODE_ROTATING    3  

#endif /*  _EZGRAPH3D_  */
