/* ------------------------------------------------------------------------- */
/* ezgraph3d.c -- Graphics routines (except for marching cubes).
 *
 * Copyright (C) 1996, 1997, 1998 Dwight Barkley and Matthew Dowle
 *
 * RCS Information
 * ---------------------------
 * $Revision: 1.1.1.1 $
 * $Date: 2003/07/05 15:08:18 $
 * ------------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> /* for unlink() */
#ifndef _GLONLY
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h> 
#include <X11/keysym.h>
#include <X11/keysymdef.h>
#include <GL/gl.h>
#include <GL/glx.h>
#else /* _GLONLY */
#define Bool unsigned char
#ifdef _MACOSX
#include <GLUT/glut.h>
#else /* !_MACOSX */
#include <GL/glut.h>
#endif /* _MACOSX */
#endif /* !_GLONLY */

#include "ezscroll.h"
#include "ezgraph3d.h"
#include "ezmarching.h"

/* -------------------------------------------------------------------------
 * This file contains all graphics manipulation routines. The functions that
 * compute the iso-surfaces and filaments and render these are in ezmarching.
 *
 * The important things to know about this file are: 
 *
 * (1) X11 is used to open the graphics window and handle the events
 * (i.e. key presses and pointer motions within the window).  InitX() is a
 * long routine that opens the window; Event_check() contains the event loop
 * that looks for window events; QuitX() quits X obviously. You should be
 * able to switch over to a higher-level method with only minor
 * modifications except for these routines.
 *
 * If option -D_GLONLY is selected in the Makefile, OpenGL is used for both
 * graphics and keyboard handling.
 *
 * (2) After the window is open, OpenGL is used to handle all the rendering.
 * myReshape() must be called before anything can be plotted.  To understand
 * this function see the OpenGL manual.
 *
 * The routines near the top of this file handle the interactive graphics
 * through OpenGL calls.  Note: to add a new interactive feature, add to the
 * event loop in Event_check() and add a corresponding routine to perform the
 * desired task.
 *
 * (3) Note on the graphics modes.
 *
 * There are 3 modes the program can be in: 
 *   MODE_SIMULATING  Simulation progresses through time (it cannot be 
 *                    rotated while in this mode)
 *   MODE_VIEWING     Simulation is paused.  While in this mode, graphics 
 *                    lists are created for use in rotations.  New list must
 *                    be created for each basic change in what is plotted,
 *                    e.g switching u-field to v-field, turning on clipping, 
 *                    etc.
 *   MODE_ROTATING    Mouse button 1 is down so the view responds to mouse 
 *                    motion.  Graphics lists are called and no new 
 *                    iso-surfaces are computed (i.e. fast rotations).
 * ------------------------------------------------------------------------- */

#ifndef _GLONLY
/* 
 * Global variables for this file only 
 * ----------------------------------- */
static Display     *theDisplay;
static Window      theWindow;
#endif /* !_GLONLY */

static int         field;            /* Field being viewed */ 
static int         show_filament;    /* Filament flag */
static int         clipping;         /* Clipping flag */
static int         ezmode;           /* Mode flag */
static int         lastx, lasty;     /* Used by Rotate() and Mouse_down() */
static GLbitfield  ezdepth;          /* Depth buffer flag */ 

static Real plot_length[3];   /* Lengths of the simulation volume in graphics
			       * coordinates (see Draw_ini()). */
static GLfloat      theta = INITIAL_THETA, 
                    phi   = INITIAL_PHI; 
/* 
 * Private functions 
 * ----------------- */
static void  Pause                    (void);
static void  Make_lists               (void);
static void  View_u_field             (void);
static void  View_v_field             (void);
static void  View_no_field            (void);
static void  Toggle_filament_plotting (void);
static void  Toggle_depth_buffer      (void);
static void  Toggle_clipping_plane    (void);
static void  Rotate                   (int x, int y, 
				       float new_theta, float new_phi);
static void  setView                  (GLfloat theta, GLfloat phi);
static void  myReshape                (int w, int h);
static void  InitX                    (int argc, char **argv, int winx, int winy, 
				       int width, int height);
static void  Save_image               (void);
#ifndef _GLONLY
static void  Restart                  (void);
static void  Mouse_up                 (void);
static void  Mouse_down               (int x, int y);
static void  Mover                    (int m_x, int m_y, int m_z);
static void  Shift_fields             (int index1, int index2);
static Bool  WaitForNotify            (Display *d, XEvent *e, char *arg);
#else /* _GLONLY */
static void myMouse                   (int button, int state, int x, int y);
static void myKey                     (unsigned char key, int x, int y);
static void myDisplay                 (void);
static void myAnimate                 (void);
static void  myChangeView             (GLfloat Delta_theta, GLfloat Delta_phi);
#endif /* !_GLONLY */
/* ========================================================================= */

void Draw (void)
{
 /* Highest level plotting routine. If in MODE_SIMULATING or
   * MODE_VIEWING then call the Marching_Cubes function.
   *
   * If in MODE_ROTATING call glCallList() which rapidly plots the new view
   * of the pre-compiled graphics scene. */

#if 0
  glClear(GL_COLOR_BUFFER_BIT | ezdepth);  /* Clear the color buffer.  
					    * Clear the depth buffer if on, 
					    * (ezdepth=DEPTH_BUFFER_BIT) */
#else
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#endif
  switch (ezmode) {
  case MODE_SIMULATING :
  case MODE_VIEWING :
    Marching_cubes(simulating_resolution,field,show_filament,clipping,
		   plot_length);
    break;
  case MODE_ROTATING :
    glCallList(ISO_LIST);
    break;    
  }
  if(GRAPHICS) 
#ifndef _GLONLY
    glXSwapBuffers(theDisplay,theWindow); 
#else /* _GLONLY */
  glutSwapBuffers();
#endif /* !_GLONLY */
}
/* ========================================================================= */

static void Make_lists (void)
{
  /* Make new graphics lists for fast rotations. */

  glNewList(ISO_LIST, GL_COMPILE);
  Marching_cubes(rotating_resolution,field,show_filament,clipping,
		 plot_length);
  glEndList();
}
/* ========================================================================= */

#ifndef _GLONLY
static void Restart (void)
{
  /* Return to MODE_SIMULATING. This has the effect of restarting the
     simulation via a return from Event_check(). */

  if (ezmode == MODE_VIEWING) {
    ezmode = MODE_SIMULATING;
  }
}
#endif /* !_GLONLY */
/* ========================================================================= */

static void Pause (void)
{
  /* The simulation has paused -- go to MODE_VIEWING.  Call Make_lists() in
   * preparation for rotation. */

  if (ezmode == MODE_SIMULATING) {
    ezmode = MODE_VIEWING;
    Make_lists();
  }
#ifdef _GLONLY
  /* In this event model we have to count for that case in which one
   * wants to go from VIEVING->SIMULATING */
  else if (ezmode == MODE_VIEWING) {
    ezmode = MODE_SIMULATING;
  }
#endif /* _GLONLY */
}
/* ========================================================================= */

static void View_u_field (void)
{
  field = U_FIELD;
  if (ezmode != MODE_SIMULATING) {
    Make_lists();
    Draw();
  }
}
/* ========================================================================= */

static void View_v_field (void)
{
  field = V_FIELD;
  if (ezmode != MODE_SIMULATING) {
    Make_lists();
    Draw();
  }
}
/* ========================================================================= */

static void View_no_field (void)
{
  field = NO_FIELD;
  if (ezmode != MODE_SIMULATING) {
    Make_lists();
    Draw();
  }
}
/* ========================================================================= */
  
static void Toggle_filament_plotting (void)
{
  if (show_filament) show_filament = FALSE;
  else show_filament = TRUE;

  if (ezmode != MODE_SIMULATING) {
    Make_lists();
    Draw();
  }
}
/* ========================================================================= */

static void Toggle_depth_buffer (void)
{
  if (glIsEnabled(GL_DEPTH_TEST)) {
    glDisable(GL_DEPTH_TEST);
    ezdepth = 0; 
  }
  else {
    glEnable(GL_DEPTH_TEST);
    ezdepth = GL_DEPTH_BUFFER_BIT;
  }
  if (ezmode != MODE_SIMULATING) {
    Draw();
  }
}
/* ========================================================================= */

static void Toggle_clipping_plane (void)
{
  if (clipping) {
    glDisable(GL_CLIP_PLANE0);
    clipping = FALSE;
  }
  else {
    glEnable(GL_CLIP_PLANE0);
    clipping = TRUE;
  }
  if (ezmode != MODE_SIMULATING) {
    Make_lists(); 
    Draw();
  }
}
/* ========================================================================= */

#ifndef _GLONLY
static void Mouse_up (void)
{
  if (ezmode == MODE_ROTATING) {
    ezmode = MODE_VIEWING;
    Draw();
  }
}
/* ========================================================================= */

static void Mouse_down (int x, int y)
{
  /* If the mode is MODE_VIEWING then on mouse down the state goes to
   * MODE_ROTATING.  x and y are the location of the mouse when the button
   * was pressed and these are saved as lastx and lasty for use in
   * rotating(). */

  if (ezmode == MODE_VIEWING) {

    lastx = x;    /* Rotate() uses these values to calculate the distance  */ 
    lasty = y;    /* moved by the mouse since the last Draw() call */

    ezmode = MODE_ROTATING;
    Draw();
  }
}
/* ========================================================================= */
#endif /* !_GLONLY */

static void Rotate (int x, int y, float new_theta, float new_phi)  
{
  /* Rotates volume either in response to changes in mouse location or to
   * angles new_theta, new_phi.  More specifically:
   *
   * if MODE_ROTATING: change the viewing angles theta and phi by the
   *   difference of (x,y) - (lastx,lasty), i.e. the difference between the
   *   current mouse location and the previous location (set in call to
   *   Rotate() or to Mouse_down()).  
   *
   * else (not in MODE_ROTATING): set theta and phi to incoming values. */

  if (ezmode == MODE_ROTATING) {
    theta += ROTATE_SCALE * (x-lastx);
    phi   -= ROTATE_SCALE * (y-lasty);
    lastx = x;
    lasty = y;
  }
  else {
    theta = new_theta;
    phi   = new_phi;
  }

  /* Require both theta and phi to be in the interval [-180,180).  Extending
   * the range of theta to [-180,180) prevents 'flipping' at theta equal 90
   * and -90. */

  if (theta > 180.0) theta -= 360.0;
  else if (theta <= -180) theta += 360.0;
  if (phi > 180.0) phi -= 360.0;
  else if (phi <= -180) phi += 360.0;

  setView(theta, phi);
}
/* ========================================================================= */

static void setView(GLfloat theta, GLfloat phi)
{
  /* OpenGL calls to set rendering (transformation matrix). Note that the
   * last call (which is the first action applied to a triangle being
   * rendered) is to center the 3D volume about the origin. There are then
   * rotations in theta and phi.  The second translation pulls the view away
   * from the cube.  I have found that DISTANCE=5 is about right -- the
   * distance (as measured in unit lengths as they appear on the screen) the
   * volume is from the eye under normal view conditions is about 5.  In
   * principle this varies with the size of the window on the screen.  For
   * an orthographic projection this is irrelevant so long as the cube is in
   * the viewed volume. */

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glTranslatef(0.0,0.0,-DISTANCE); 
  glRotatef(-phi,1.0,0.0,0.0);
  glRotatef(theta,0.0,0.0,1.0);
  glTranslatef(-plot_length[0]/2,-plot_length[1]/2,-plot_length[2]/2);
}
/* ========================================================================= */

static void myReshape(int w, int h)
{
  GLfloat half_width;

  /* half_width is the half width of the volume viewed in GL.  In general if
   * half_width is large, then rendered simulation volume will appear small,
   * and vice versa.  PLOT_SIZE in ezgraph3d.h allows adjustment of this
   * without changing any of the code below.
   *
   * For orthographic projection setting half_width this is straightforward:
   * The simulation volume has maximum side of 1 in graphics coordinates
   * (set Draw_ini()).  If half_width is set to sqrt(3)/2 = 0.866..., then a
   * unit cube, and hence the simulation volume, will always lie entirely
   * within the viewport so I choose this for the default.  For ortho
   * projection, the near are far values are set to large values and are
   * irrelevant so long as the volume lies between near and far.
   * 
   * For perspective projection the situation is more complicated because
   * the half_width applies at the value of near (see the OpenGL manual).
   * Hence the value of near and half_width must be set together so that a
   * unit cube fills the viewport but does not cut the near plane.  The
   * values below accomplish this.  The far value if irrelevant and is set to
   * a large value. */

  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();
#if PERSPECTIVE   
  /* Perspective projection */
  half_width = 0.7/PLOT_SIZE;
  glFrustum(-half_width, half_width, -half_width, half_width, 
	    DISTANCE-1., 20.);
#else            
  /* Orthographic projection */
  half_width = 0.866/PLOT_SIZE;
  glOrtho (-half_width, half_width, -half_width, half_width, -20., 20.); 
#endif
  glMatrixMode (GL_MODELVIEW);
  glViewport (0, 0, w, h);
}
/* ========================================================================= */

void Draw_ini (int argc, char **argv, int initial_field)
{
  /* Initialize everything necessary for 3D plotting.  */

  field = initial_field;

  show_filament = FALSE;

  /* The lengths of the simulation volume in graphics coordinates are set.  I
   * choose to have the largest plot_length=1.  Thus the volume lies inside
   * the unit cube in graphics coordinates. */
  {
    int nmax = max(max(NX,NY),NZ);
    plot_length[0] = (NX-1.)/(nmax-1.);
    plot_length[1] = (NY-1.)/(nmax-1.);
    plot_length[2] = (NZ-1.)/(nmax-1.);
  }

  /* Initialization for marching cubes. */
  Marching_ini();

  /* At this point everything has been initialized for finding filaments
   * without graphics.  Can return after setting a few things.  Setting field
   * to NO_FIELD and ezmode to MODE_SIMULATING will give only minimum OpenGL
   * calls.  clipping must be set but is not important. */

  if( !GRAPHICS ) {
    field = NO_FIELD;
    ezmode = MODE_SIMULATING;   
    clipping = FALSE;
    return;
  }

  /* Create X window and prepares for use by OpenGL */
  InitX(argc, argv, WINX,WINY,WINSIZE,WINSIZE);

  /* Set the shade model to Flat. The default Smooth (GL_SMOOTH) 
   * can take too long on some systems and I see no difference. */
  glShadeModel(GL_FLAT); 

  /* Makes little difference. */
  glEnable(GL_DITHER); 

  /* The depth buffer is disabled by default in GL.  Here we enable
   * it. To start with depth buffer disabled, don't call glEnable and
   * set ezdepth = 0. */
  glEnable(GL_DEPTH_TEST);
  ezdepth = GL_DEPTH_BUFFER_BIT;

  /* Set the background color.  Here: red=green=blue=BACKGROUND.  One can
   * set any values one chooses so long as 0 <= red, green, blue <=1 */
  glClearColor(BACKGROUND,BACKGROUND,BACKGROUND,0.0); 

  /* Set up the clipping plane.  Note that clipping is not on enabled
   * initially, but could be by inserting a call to Toggle_clipping_plane().
   * Here the clipping plane is set to pass half way through the volume.
   * plane[3] controls this.  I start with clipping off (clipping=FALSE) */
  {
    GLdouble plane[4]; 
    plane[0] = 0.0; 
    plane[1] = 0.0;
    plane[2] = -1.0;
    plane[3] = plot_length[2]/2;
    setView(0., 0.);
    glClipPlane(GL_CLIP_PLANE0, plane);
    clipping = FALSE;
  }

  /* Initialize the view direction */
  Rotate(0, 0, (float)INITIAL_THETA, (float)INITIAL_PHI);   

  /* Set starting mode */
  if(START_PAUSED) {
    ezmode = MODE_VIEWING; 
    Make_lists();
  }
  else {
    ezmode = MODE_SIMULATING;   
  }

}
/* ========================================================================= */

#ifndef _GLONLY
static void Mover (int m_x, int m_y, int m_z)
{
  /* Moves scrolls in response to a key press */

  int i, i_start, i_stop, i_step;
  int j, j_start, j_stop, j_step;
  int k, k_start, k_stop, k_step;
  
  /* Check for out-of-range. It is necessary to move at most one grid point
   * in each direction to avoid problems at domain boundaries. */

  if( (abs(m_x)>1)||(abs(m_y)>1)||(abs(m_z)>1) ) {
    fprintf(stderr,"out-of-range in Mover(): m_x, m_y, m_z = %d, %d, %d\n",
	    m_x, m_y, m_z);
    exit(1);
  }

  if(m_x > 0) {i_start = NX; i_stop = 1;  i_step = -1;}
  else        {i_start = 1;  i_stop = NX; i_step =  1;}
  
  if(m_y > 0) {j_start = NY; j_stop = 1;  j_step = -1;}
  else        {j_start = 1;  j_stop = NY; j_step =  1;}
  
  if(m_z > 0) {k_start = NZ; k_stop = 1;  k_step = -1;}
  else        {k_start = 1;  k_stop = NZ; k_step =  1;}
  
  for(i=i_start;i!=(i_stop+i_step);i+=i_step) {
    for(j=j_start;j!=(j_stop+j_step);j+=j_step) {
      for(k=k_start;k!=(k_stop+k_step);k+=k_step) {
	Shift_fields(INDEX(i,j,k), INDEX(i+m_x,j+m_y,k+m_z));
      }
    }
  }

  /* Wrap-around for periodic bcs */

#if PBC_x
  if(m_x!=0) {
    for(j=j_start;j!=(j_stop+j_step);j+=j_step) {
      for(k=k_start;k!=(k_stop+k_step);k+=k_step) {
	if(m_x>0) 
	  Shift_fields(INDEX(NX,j,k), INDEX(1,j+m_y,k+m_z));
	else 
	  Shift_fields(INDEX(1,j,k), INDEX(NX,j+m_y,k+m_z));
      }
    }
  }
#endif

#if PBC_y
  if(m_y!=0) {
    for(i=i_start;i!=(i_stop+i_step);i+=i_step) {
      for(k=k_start;k!=(k_stop+k_step);k+=k_step) {
	if(m_y>0)
	  Shift_fields(INDEX(i,NY,k),INDEX(i+m_x,1,k+m_z));
	else 
	  Shift_fields(INDEX(i,1,k), INDEX(i+m_x,NY,k+m_z));
      }
    }
  }
#endif

#if PBC_z
  if(m_z!=0) {
    for(i=i_start;i!=(i_stop+i_step);i+=i_step) {
      for(j=j_start;j!=(j_stop+j_step);j+=j_step) {
	if(m_z>0) 
	  Shift_fields(INDEX(i,j,NZ), INDEX(i+m_x,j+m_y,1));
	else 
	  Shift_fields(INDEX(i,j,1), INDEX(i+m_x,j+m_y,NZ));
      }
    }
  }
#endif

  if (ezmode != MODE_SIMULATING) {
    Make_lists();
    Draw();
  }
}
/* ========================================================================= */
#endif /* !_GLONLY */

#ifndef _GLONLY
void Shift_fields (int index1, int index2)
{
  /* There is unnecessary copying of the spatial sums here.  If s1 and s2
   * from ezstep3d were known, then the unnecessary copying could be
   * eliminated.  I don't move the scrolls that much so prefer to keep the
   * code simpler. */

  u[index2] = u[index1];  
  v[index2] = v[index1]; 
  sigma_u[index2] = sigma_u[index1]; 
  sigma_u[FIELD_SIZE+index2] = sigma_u[FIELD_SIZE+index1]; 
#if V_DIFF_ON 
  sigma_v[index2] = sigma_v[index1]; 
  sigma_v[FIELD_SIZE+index2] = sigma_v[FIELD_SIZE+index1]; 
#endif
}
/* ========================================================================= */
#endif /* !_GLONLY */

int Event_check (void)
{
#ifndef _GLONLY
  static XEvent         theEvent;
  static KeySym         theKeySym;
  static int            theKeyBufferMaxLen = 64;
  static char           theKeyBuffer[65];
  static XComposeStatus theComposeStatus;
  int                   write_filament_save;

  if( !GRAPHICS ) return(0);

  /* Save write_filament, and then turn filament writing is turned off.  This
   * is done so that no filaments are written as a result of graphics calls
   * in the event loop.  At the end, the write_filament is restored. */

  write_filament_save = write_filament;
  write_filament = FALSE;

  /* X Event Loop
   *
   * If there is something in the queue then each event is processed in
   * turn. When the queue is empty, and the mode (which may have been changed
   * by the events just processed) is MODE_SIMULATING then control is
   * returned (presumably to the main loop in main()).  However, when the
   * queue is empty and the mode is either MODE_ROTATING or MODE_VIEWING then
   * XNextEvent is called which blocks until the next X event is received
   * (eg. the space bar being pressed which sets the mode back to
   * MODE_SIMULATING and so control returns to main()). */

  while(XPending(theDisplay) || (ezmode != MODE_SIMULATING)) {

    XNextEvent(theDisplay, &theEvent);
    
    switch(theEvent.type) {      /* switch according to X event */
      
    case KeyPress:               /* A KeyPress event has occurred. */
      
      XLookupString((XKeyEvent *) &theEvent, theKeyBuffer,
		    theKeyBufferMaxLen, &theKeySym, &theComposeStatus);

      switch(theKeySym) {        /* switch according to the pressed key */

      case XK_Escape: exit(0);   /* hard exit, nothing saved */

      case XK_Q:
      case XK_q:      return(1);

      case XK_P:
      case XK_p:      Pause(); Draw(); break;
	
      case XK_space:  Restart(); break;
	
      case XK_C:
      case XK_c:      Toggle_clipping_plane(); break;
	
      case XK_D:
      case XK_d:      Toggle_depth_buffer(); break;
	
      case XK_F:
      case XK_f:      Toggle_filament_plotting(); break;
	
      case XK_U:
      case XK_u:      View_u_field(); break;
	
      case XK_V:
      case XK_v:      View_v_field(); break;
	
      case XK_N:
      case XK_n:      View_no_field(); break;

      case XK_S:
      case XK_s:      Save_image(); break;

      case XK_Right:  Mover(1,0,0);  break;    /* move +x */
      case XK_Left:   Mover(-1,0,0); break;    /* move -x */
      case XK_Up:     Mover(0,1,0);  break;    /* move +y */
      case XK_Down:   Mover(0,-1,0); break;    /* move -y */
      case XK_plus:   Mover(0,0,1);  break;    /* move +z */
      case XK_minus:  Mover(0,0,-1); break;    /* move -z */

	/* reset view to initial view */
      case XK_R:      
      case XK_r:      Rotate(0,0,INITIAL_THETA,INITIAL_PHI); Draw(); break; 

	/* Set view to looking down the z-axis (z decreasing away from eye)
	 * with x-y in usual orientation.  Useful view for moving scrolls */
      case XK_Z:      
      case XK_z:      Rotate(0,0,0.,0.); Draw(); break;

      }  /* switch(theKeySym) */
      break;

    case EnterNotify:
      /* This case is necessary for window managers which do not set keyboard
       * focus to theWindow when the pointer enters theWindow. */
      XSetInputFocus(theDisplay, theWindow, RevertToPointerRoot, CurrentTime); 
      break;
      
    case Expose:
      /* Window mapped for the first time and if you uncover some part of the
       * window. If you start paused and you see nothing in the window then
       * its possible that the problem is that the first Expose event is not
       * being caught for some reason. */
      Draw();
      break;

    case ConfigureNotify:
      /* Window size is changed by the user (or the window is initially
       * opened). Note that InitX contains code that constrains the window
       * to be square. */
      myReshape(theEvent.xconfigure.width, theEvent.xconfigure.height);
      break;

    case ButtonPress:
      if (theEvent.xbutton.button == 1) {
	Mouse_down(theEvent.xbutton.x, theEvent.xbutton.y);
      }
      break;

    case ButtonRelease:
      if (theEvent.xbutton.button == 1) {
	Mouse_up();
      }
      break;

    case MotionNotify:
      if (theEvent.xmotion.state & Button1Mask) {
	/* Remove all the MotionNotify events currently in the queue leaving
	 * the last one in theEvent. This ensures the image 'sticks' to the
	 * mouse. */
	while (XCheckWindowEvent(theDisplay,theWindow,
				 PointerMotionMask, &theEvent));
	Rotate(theEvent.xmotion.x, theEvent.xmotion.y, 0., 0.);
	Draw(); 
      }
      break;

    } /* switch (theEvent.type) */

  } /* while (XPending(theDisplay) || (ezmode != MODE_SIMULATING)) */

  /* restore write_filament */
  write_filament = write_filament_save;
#endif /* !_GLONLY */
  return(0);
}
/* ========================================================================= */

static void InitX (int argc, char **argv, int winx, int winy, int width, int height)
{
#ifndef _GLONLY
  /* Initializes X and opens a window. */

  static XVisualInfo           *theVisualInfo;
  static GLXContext            theGLXContext;
  static Colormap              theColormap;
  static int                   theScreen; 
  static int                   theDepth;
  static int                   theDWidth;
  static int                   theDHeight;
  static char                  *theDisplayName = NULL;
  static XEvent                event;
  static Atom                  del_atom;
  static XSizeHints            theSizeHints;
  static XSetWindowAttributes  theSWA;
  static char                  *name = WINDOW_TITLE;
  static XTextProperty         theWindowName, theIconName;
  static int                   num1,num2;
  static int list[] = {GLX_RGBA,
		       GLX_DOUBLEBUFFER,
		       GLX_RED_SIZE, 1,
		       GLX_GREEN_SIZE, 1,
		       GLX_BLUE_SIZE, 1,
		       GLX_DEPTH_SIZE, 1,
		       None } ;

  /* DEPTH and DOUBLE are the important ones.  Perhaps need to add error
   * checking when DOUBLE_BUFFER and DEPTH not available.  In the aux library
   * the first entry in list was GLX_LEVEL, 0, */

  /* Open the display */

  if ((theDisplay = XOpenDisplay(NULL)) == NULL) {
    fprintf(stderr, 
	    "ERROR: Could not open a connection to X on display %s\n",
	    XDisplayName(theDisplayName));
    exit(1);
  }

  if (!glXQueryExtension(theDisplay, &num1, &num2)) {
    fprintf(stderr,
	    "ERROR: No glx extension on display %s\n",
	    XDisplayName(theDisplayName));
    exit(1);
  }

  theScreen     = DefaultScreen(theDisplay);
  theDepth      = DefaultDepth (theDisplay, theScreen);
  theDWidth     = DisplayWidth (theDisplay, theScreen);
  theDHeight    = DisplayHeight(theDisplay, theScreen);

  if (!(theVisualInfo = glXChooseVisual(theDisplay, theScreen, list))) {
    fprintf(stderr, "ERROR: Couldn't find visual");
    exit(-1);
  }
  if (!(theGLXContext = glXCreateContext(theDisplay, theVisualInfo,
					 None, GL_TRUE))) {
    /* Last parameter indicates that, if possible, then render directly to
     * graphics hardware and bypass the X server. This should be faster. */
    fprintf(stderr, "ERROR: Can not create a context!\n");
    exit(-1);
  }

  theColormap = XCreateColormap(theDisplay,
				RootWindow(theDisplay, theVisualInfo->screen),
				theVisualInfo->visual, AllocNone);
  /* AllocAll would generate a BadMatch.  */

  if (!(theColormap)) {
    fprintf(stderr, "ERROR: couldn't create Colormap\n");
    exit(-1);
  }
  theSWA.colormap = theColormap;
  theSWA.border_pixel = 0;
  theSWA.event_mask = (EnterWindowMask | KeyPressMask | StructureNotifyMask | 
		       ButtonPressMask | ButtonReleaseMask | ExposureMask | 
		       PointerMotionMask);

  theWindow = XCreateWindow(theDisplay,
			    RootWindow(theDisplay, theVisualInfo->screen),
			    winx, winy, width, height, 0,
			    theVisualInfo->depth, InputOutput,
			    theVisualInfo->visual,
			    CWBorderPixel|CWColormap|CWEventMask, &theSWA);

  if (!(theWindow)) {
    fprintf(stderr, "ERROR: couldn't create X window\n");
    exit(-1);
  }

  /* Set standard window properties.  theWindowName and theIconName are set
   * to Name, which unless you change it (in ezgraph3d.h), it will be
   * EZ-Scroll. */

  XStringListToTextProperty(&name,1,&theWindowName);
  XStringListToTextProperty(&name,1,&theIconName);

  theSizeHints.base_width = width;
  theSizeHints.base_height = height;
  theSizeHints.min_aspect.x = width;   /* Maintain x:y ratio */
  theSizeHints.max_aspect.x = width;
  theSizeHints.min_aspect.y = height;
  theSizeHints.max_aspect.y = height;

  theSizeHints.flags = PSize|PAspect;

  if(!(WM_CTRLS_POS)) theSizeHints.flags |= USPosition;
  /* Setting USPosition here seems to make the WM honor the x and y
   * specified by XCreateWindow above.  Not setting this should give control
   * of position to the WM.  Note that the root window of an application is
   * special in that the WM has special privileges over its properties so
   * this may not work on all platforms.  */

  XSetWMProperties(theDisplay, theWindow, &theWindowName, &theIconName,
		   NULL, 0, &theSizeHints, NULL, NULL);

  /* Express interest in WM killing this application  */
  if ((del_atom = XInternAtom(theDisplay, "WM_DELETE_WINDOW", TRUE)) != None) {
    XSetWMProtocols(theDisplay, theWindow, &del_atom, 1); 
  }

  XMapWindow(theDisplay, theWindow);
  XIfEvent(theDisplay, &event, WaitForNotify, (char *)theWindow);

  glXMakeCurrent(theDisplay, theWindow, theGLXContext);

  /* Print useful information. I suggest printing this at this at least once */
  if(verbose>1) {
    printf("%s version %d of the X Window System, X%d R%d\n",
	   ServerVendor    (theDisplay),
	   VendorRelease   (theDisplay),
	   ProtocolVersion (theDisplay),
	   ProtocolRevision(theDisplay));

    if(theDepth==1) {
      printf("Color plane depth...........%d (monochrome)\n", theDepth);
    }
    else {
      printf("Color plane depth...........%d \n",             theDepth);
    }

    printf("Display Width...............%d \n", theDWidth);
    printf("Display Height..............%d \n", theDHeight);
    printf("The display %s\n", XDisplayName(theDisplayName));
  }
#else /* _GLONLY */
  ezmode = MODE_SIMULATING; /* until keyboard/mouse events implemented */
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize(width, height); 
  glutInitWindowPosition (100, 100);
  glutCreateWindow(WINDOW_TITLE);
  glutDisplayFunc(myDisplay);
  glutReshapeFunc(myReshape);
  glutKeyboardFunc(myKey);
  glutMouseFunc(myMouse);
  glutIdleFunc(myAnimate);
  Rotate(0, 0, theta, phi);
  printf("OpenGL graphics initialised - entering OpenGL main loop\n\n");
  glutMainLoop();
#endif /* !_GLONLY */
}
/* ========================================================================= */

#ifndef _GLONLY
static Bool WaitForNotify(Display *d, XEvent *e, char *arg) 
{
  /*  As seen in the Porting Guide. */
  return (e->type == MapNotify) && (e->xmap.window == (Window)arg);
}
#endif /* !_GLONLY */
/* ========================================================================= */

void QuitX (void)
{
#ifndef _GLONLY
  if(GRAPHICS)
    XCloseDisplay(theDisplay);
#else
  /* GL/GLUT cleanup needed? */ 
#endif
}
/* ========================================================================= */

static void Save_image (void)
{

  static int num = 1;
  char buf[128]; /* MAX_NAME */
#if 0  
  /* Save to a file an rgb image of what is currently on the screen. Usable
     only on SGI. */
  glFlush();
  sprintf(buf, "scrsave snap%d.rgb %d %d %d %d", 
	  num++, WINX+8, WINX+WINSIZE-8, 1024-WINY-40, 1024-(WINY+WINSIZE));
  system(buf);
#elif defined(_GLONLY)
  GLubyte *img;
  FILE *fp;

  glPixelZoom(1.0, 1.0);
  glRasterPos2i(0, 0);
  img = (GLubyte *)malloc(WINSIZE * WINSIZE * 3 * sizeof(GLubyte));
  if (!img) {
    fprintf(stderr, "Save_image: Malloc of %ld bytes failed.\n", WINSIZE * WINSIZE * 3 * sizeof(GLubyte));
    exit(1);
  } 
  glReadPixels(0, 0, WINSIZE, WINSIZE, GL_RGB, GL_UNSIGNED_BYTE, img);
  sprintf(buf, "snap%d.pgm", num);
  fp = fopen(buf, "wb");
  if (fp) {
    /* Write the PGM header */
    fprintf(fp, "P6\n%d %d\n255\n", WINSIZE, WINSIZE);
    /* Write the image data */
    if ((fwrite(img, (WINSIZE * WINSIZE * 3), 1, fp)) != 1) {
      fprintf(stderr, "Save_image: Saving screenshot %d failed: file I/O error.\n", num);
      fclose(fp);
    } else {
      fclose(fp);
      sprintf(buf, "cjpeg -quality 95 snap%d.pgm > snap%d.jpg", num, num);
      if (system(buf) != 0) {
	fprintf(stderr, "Save_image: PGM conversion failed\n");
      } else {
	/* unlink i.e. remove pgm file */
	sprintf(buf, "snap%d.pgm", num);
	unlink(buf);
      }
    }
  } else {
    fprintf(stderr, "Save_image: Saving screenshot %d failed, can not open file.\n", num);
  }
  free(img);
  num++;
#endif /* defined(_GLONLY) */
}
/* ========================================================================= */

/* Below are function needed in GL-only version (-D_GLONLY) */
#ifdef _GLONLY

static void myMouse (int button, int state, int x, int y)
{
    if (state == GLUT_DOWN) {
      switch (button) {
      case GLUT_LEFT_BUTTON:
	if (verbose > 2)
	  printf("Left buttonpress (%d,%d)\n", x, y);
	break;
      case GLUT_MIDDLE_BUTTON:
	break;
      case GLUT_RIGHT_BUTTON:
	break;
      } /* end switch(button) */
    } /* end state == GLUT_DOWN */
}
/* ========================================================================= */

static void myChangeView (GLfloat Delta_theta, GLfloat Delta_phi)
{
  theta += Delta_theta;
  phi += Delta_phi;

  if (theta > 180.0) theta -= 360.0;
  else if (theta <= -180) theta += 360.0;
  if (phi > 180.0) phi -= 360.0;
  else if (phi <= -180) phi += 360.0;

  setView(theta, phi);
  Draw();
}
/* ========================================================================= */

static void myDisplay(void)
{
  Draw();
 /*if( (istep%plot_step) == 0 ) Draw(); ??? */
}
/* ========================================================================= */

static void myAnimate(void)
{
  if (istep < nsteps) {
    /*if( Event_check() )          break;*/
    if( hist_step && (istep%hist_step)==0 ) Write_history(istep);
    Step();
    if( (istep%plot_step) == 0 ) Draw();
    istep++;
  } else {
    Finish_up();
    exit(0);
  }
}
/* ========================================================================= */

static void myKey(unsigned char key, int x, int y)
{
  switch (key) {

  case 'H':
  case 'h':
    /* vi-arrow key left */
    if (ezmode == MODE_VIEWING) 
      myChangeView(-DELTA_VIEW_DEG, 0.0);
    break;

  case 'L':
  case 'l':
    /* vi-arrow key right */
    if (ezmode == MODE_VIEWING) 
      myChangeView(DELTA_VIEW_DEG, 0.0);
    break;

  case 'J':
  case 'j':
    /* vi-arrow key up */
    if (ezmode == MODE_VIEWING) 
      myChangeView(0.0, DELTA_VIEW_DEG);
    break;

  case 'K':
  case 'k':
    /* vi-arrow key down */
    if (ezmode == MODE_VIEWING) 
      myChangeView(0.0, -DELTA_VIEW_DEG);
    break;

  case ' ':
      case 'P':
      case 'p': 
	Pause(); 
	Draw();
	switch (ezmode) {
	case MODE_SIMULATING:
	  glutIdleFunc(myAnimate);
	  if (verbose > 2) 
	    printf("Simulation continued\n");
	  break;
	case MODE_VIEWING:
	  glutIdleFunc(NULL);
	  if (verbose > 2) 
	    printf("Simulation paused\n");
	  break;
	}
	break;
	
  case 'C':
  case 'c': 
    Toggle_clipping_plane(); 
    break;
	
  case 'D':
  case 'd': 
    Toggle_depth_buffer(); 
    break;
	
  case 'F':
  case 'f':  
    Toggle_filament_plotting(); 
    break;
	
  case 'U':
  case 'u':      
    View_u_field(); 
    break;	

  case 'V':
  case 'v':      
    View_v_field(); 
    break;	

  case 'N':
  case  'n':      
    View_no_field(); 
    break;

  case 'S':
  case 's':      
    Save_image(); 
    break;

#if 0
      case XK_Right:  Mover(1,0,0);  break;    /* move +x */
      case XK_Left:   Mover(-1,0,0); break;    /* move -x */
      case XK_Up:     Mover(0,1,0);  break;    /* move +y */
      case XK_Down:   Mover(0,-1,0); break;    /* move -y */
      case XK_plus:   Mover(0,0,1);  break;    /* move +z */
      case XK_minus:  Mover(0,0,-1); break;    /* move -z */
#endif

	/* reset view to initial view */
  case 'R':      
  case 'r':      
    Rotate(0,0,INITIAL_THETA,INITIAL_PHI); 
    Draw(); 
    break;
 
    /* Set view to looking down the z-axis (z decreasing away from eye)
     * with x-y in usual orientation.  Useful view for moving scrolls */
  case 'Z':
  case 'z':
    Rotate(0,0,0.,0.); 
    Draw(); 
    break;
  case 't':    
    break;
  case 27:           
    /* Esc will quit */
    exit(1);
    break;
  }
}
/* ========================================================================= */

#endif /* _GLONLY */

