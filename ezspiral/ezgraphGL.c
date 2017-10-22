/* ------------------------------------------------------------------------- */
/* ezgraphGL.c -- OpenGL graphics routines.  
 *
 * Copyright (C) 1992, 1994, 1997, 1998 Dwight Barkley                 
 *
 * RCS Information
 * ---------------------------
 * $Revision: 3.1.1.2 $
 * $Date: 2002/08/01 08:17:14 $
 * ------------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h> 
#include <X11/keysym.h>
#include <X11/keysymdef.h>
#include <GL/gl.h>
#include <GL/glx.h>

#include "ezspiral.h"
#include "ezgraphGL.h"

/* -------------------------------------------------------------------------
 * This file contains all graphics manipulation routines. 
 * The important things to know about this file are: 
 *
 * (1) X11 is used to open the graphics window and handle the events
 * (i.e. key presses and pointer motions within the window).  InitX() is a
 * long routine that opens the window; Event_check() contains the event loop
 * that looks for window events; QuitX() quits X obviously. You should be
 * able to switch over to a higher-level method with only minor
 * modifications except for these routines.
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
 * There are 2 modes the program can be in: 
 *   MODE_SIMULATING  Simulation progresses through time (it cannot be 
 *                    rotated while in this mode)
 *   MODE_VIEWING     Simulation is paused.  
 * ------------------------------------------------------------------------- */

/* 
 * Global variables for this file only 
 * ----------------------------------- */
static Display     *theDisplay;
static Window      theWindow;

static int         field;            /* Field being viewed */ 
static int         show_tip;         /* Tip draw flag */
static int         ezmode;           /* Mode flag */

static Real        half_width, 
                   half_height;

static Real        plot_length[2];   /* Lengths of the simulation volume in
					graphics coordinates (see
					Draw_ini()). */
/* 
 * Private functions 
 * ----------------- */
static int   setColor             (int i, int j);
static void  Draw_tips            (void);
static void  Restart              (void);
static void  Pause                (void);
static void  View_u_field         (void);
static void  View_v_field         (void);
static void  View_no_field        (void);
static void  Toggle_tip_plotting  (void);
static void  myReshape            (int w, int h);
static void  Mover                (int m_x, int m_y);
static void  Shift_fields         (int index1, int index2);
static void  InitX                (int winx, int winy, int width, int height);
static Bool  WaitForNotify        (Display *d, XEvent *e, char *arg);
static void  Save_image           (void);

/* ========================================================================= */

void Draw (void)
{
  /* Main plotting routine */

  int i,j;
  Real x1, x2, y1, y2, rect_h=plot_length[0]/(NX-1);

  if(GRAPHICS) {
    /* Clear the color buffer and draw a blue rectangle the size of
       the simulation area */
    glClear(GL_COLOR_BUFFER_BIT);    
    GLCOLOR3(0.,0.,1.);
    GLRECT(-half_width, -half_height, half_width, half_height); 
  }

  if(field != NO_FIELD) {
    y1 = -half_height;  
    y2 = y1 + rect_h;
    for(j=1;j<NY;j++) {
      x1 = -half_width;
      x2 = x1 + rect_h;
      for(i=1;i<NX;i++) {
	if(setColor(i,j)) GLRECT(x1,y1,x2,y2);
	x1 += rect_h; 
	x2 += rect_h;
      }
      y1 += rect_h; 
      y2 += rect_h;
    }
  }

  if( write_tip || show_tip ) Find_tips();
  if( show_tip ) Draw_tips(); 

  if(GRAPHICS) 
    glXSwapBuffers(theDisplay,theWindow); 
}
/* ========================================================================= */

static int setColor (int i, int j)
{
  Real scaled_v, red, green, blue;

  switch (field) {

  case U_FIELD :            /* Set the u color */
    if (U(i,j) < 0.1) {     /* quiescent state (u<0.1): no color set */
      return(0);
    }
    else if(U(i,j) < 0.9) { /* Interface (0.1<u<0.9): Black */
      GLCOLOR3(0.,0.,0.);
    }
    else {                  /* Excited state (u>0.9): Red */
      GLCOLOR3(1.,0.,0.);
    }
    break;

  case V_FIELD :            /* Set the v_color */

    /* Scale the v-field between 0 and 1.  The maximum (VMAX) and minimum
     * (VMIN) values used for scaling are somewhat ad hoc.  The max and min
     * functions are used to ensure 0 <= scaled_v < 1.  */

#define VMAX (0.8*a_param) 
#define VMIN 0.0 
    scaled_v = (V(i,j)-VMIN)/(VMAX-VMIN);
#if 0
    scaled_v = max(0.,scaled_v);
    scaled_v = min(1.,scaled_v);
#endif

    red   = 1.-scaled_v*scaled_v;
    green = scaled_v*scaled_v/2.0;
    blue  = scaled_v*scaled_v;

    GLCOLOR3(red, green, blue);
  }
  return(1);
}
/* ========================================================================= */

static void Draw_tips (void) 
{
  Real rect_h=plot_length[0]/(NX-1);
  int i;

  glLineWidth(TIP_WT);
  glBegin(TIP_PLOT_TYPE); 
  GLCOLOR3(TIP_R, TIP_G, TIP_B);
  for(i=0;i<ntips;i++) {
    GLVERTEX2(PX(tip_x[i]),PY(tip_y[i]));
  }
  glEnd();
}
/* ========================================================================= */

static void Restart (void)
{
  /* Return to MODE_SIMULATING. This has the effect of restarting the
     simulation via a return from Event_check(). */

  if (ezmode == MODE_VIEWING) {
    ezmode = MODE_SIMULATING;
  }
}
/* ========================================================================= */

static void Pause (void)
{
  if (ezmode == MODE_SIMULATING) {
    ezmode = MODE_VIEWING;
  }
}
/* ========================================================================= */

static void View_u_field (void)
{
  field = U_FIELD;
  if (ezmode != MODE_SIMULATING) {
    Draw();
  }
}
/* ========================================================================= */

static void View_v_field (void)
{
  field = V_FIELD;
  if (ezmode != MODE_SIMULATING) {
    Draw();
  }
}
/* ========================================================================= */

static void View_no_field (void)
{
  field = NO_FIELD;
  if (ezmode != MODE_SIMULATING) {
    Draw();
  }
}
/* ========================================================================= */
  
static void Toggle_tip_plotting (void)
{
  if (show_tip) show_tip = FALSE;
  else {
    show_tip = TRUE;
    ntips = 0;         /* set to zero to "erase" previous tips */
  }

  if (ezmode != MODE_SIMULATING) {
    Draw();
  }
}
/* ========================================================================= */

static void myReshape(int w, int h)
{
  /* half_width and half_height define the area viewed in GL.  In general if
   * these are large, then simulation area will appear small, and vice versa.
   * PLOT_SIZE in ezgraphGL.h allows adjustment of this without changing any
   * of the code below. */

  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();
  glOrtho (-PLOT_SIZE*half_width,  PLOT_SIZE*half_width, 
	   -PLOT_SIZE*half_height, PLOT_SIZE*half_height, -20., 20.); 
  glMatrixMode (GL_MODELVIEW);
  glViewport (0, 0, w, h);
}
/* ========================================================================= */

void Draw_ini (int initial_field)
{
  /* Initialize everything necessary for plotting.  */

  field = initial_field;

  show_tip = FALSE;

  /* The lengths of the simulation area in graphics coordinates are set.  I
   * choose to have the largest plot_length=1.  Thus the simulation area lies
   * inside the unit square in graphics coordinates. */
  {
    int nmax = max(NX,NY);
    plot_length[0] = (NX-1.)/(nmax-1.);
    plot_length[1] = (NY-1.)/(nmax-1.);
  }
  
  half_width  = 0.5*plot_length[0];
  half_height = 0.5*plot_length[1];

  /* At this point everything has been initialized for finding tips
   * without graphics.  Can return after setting a few things.  Setting field
   * to NO_FIELD and ezmode to MODE_SIMULATING will give only minimum OpenGL
   * calls. */

  if( !GRAPHICS ) {
    field = NO_FIELD;
    ezmode = MODE_SIMULATING;   
    return;
  }

  /* Create X window and prepares for use by OpenGL */
  InitX(WINX,WINY,WINSIZE*plot_length[0],WINSIZE*plot_length[1]);

  /* Set the shade model to Flat. The default Smooth (GL_SMOOTH) 
   * can take too long on some systems and I see no difference. */
  glShadeModel(GL_FLAT); 

  /* Makes little difference. */
  glEnable(GL_DITHER); 

  /* Set the background color.  Here: red=green=blue=BACKGROUND.  One can
   * set any values one chooses so long as 0 <= red, green, blue <=1 */
  glClearColor(BACKGROUND,BACKGROUND,BACKGROUND,0.0); 

  /* Set starting mode */
  if(START_PAUSED) {
    ezmode = MODE_VIEWING; 
  }
  else {
    ezmode = MODE_SIMULATING;   
  }

}
/* ========================================================================= */

void Mover (int m_x, int m_y)
{
  /* Moves spiral in response to a key press */

  int i, i_start, i_stop, i_step;
  int j, j_start, j_stop, j_step;
  
  /* Check for out-of-range. It is necessary to move at most one grid point
   * in each direction to avoid problems at domain boundaries. */

  if( (abs(m_x)>1)||(abs(m_y)>1) ) {
    fprintf(stderr,"out-of-range in Mover(): m_x, m_y = %d, %d\n", m_x, m_y);
    exit(1);
  }

  if(m_x > 0) {i_start = NX; i_stop = 1;  i_step = -1;}
  else        {i_start = 1;  i_stop = NX; i_step =  1;}
  
  if(m_y > 0) {j_start = NY; j_stop = 1;  j_step = -1;}
  else        {j_start = 1;  j_stop = NY; j_step =  1;}
  
  for(i=i_start;i!=(i_stop+i_step);i+=i_step) {
    for(j=j_start;j!=(j_stop+j_step);j+=j_step) {
      Shift_fields(INDEX(i,j), INDEX(i+m_x,j+m_y));
    }
  }

  /* Wrap-around for periodic bcs */

#if PBC_x
  if(m_x!=0) {
    for(j=j_start;j!=(j_stop+j_step);j+=j_step) {
      if(m_x>0) 
	Shift_fields(INDEX(NX,j), INDEX(1,j+m_y));
      else 
	Shift_fields(INDEX(1,j), INDEX(NX,j+m_y));
    }
  }
#endif

#if PBC_y
  if(m_y!=0) {
    for(i=i_start;i!=(i_stop+i_step);i+=i_step) {
      if(m_y>0)
	Shift_fields(INDEX(i,NY),INDEX(i+m_x,1));
      else 
	Shift_fields(INDEX(i,1), INDEX(i+m_x,NY));
    }
  }
#endif

  if (ezmode != MODE_SIMULATING) {
    Draw();
  }
}
/* ========================================================================= */

void Shift_fields (int index1, int index2)
{
  /* There is unnecessary copying of the spatial sums here.  If s1 and s2
   * from ezstep were known, then the unnecessary copying could be
   * eliminated.  I don't move the spiral that much so prefer to keep the
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

int Event_check (void)
{
  static XEvent         theEvent;
  static KeySym         theKeySym;
  static int            theKeyBufferMaxLen = 64;
  static char           theKeyBuffer[65];
  static XComposeStatus theComposeStatus;
  int                   write_tip_save;

  if( !GRAPHICS ) return(0);

  /* Save write_tip, and then turn tip writing is turned off.  This is
   * done so that no tips are written as a result of graphics calls in
   * the event loop.  At the end, the write_tip is restored. */

  write_tip_save = write_tip;
  write_tip = FALSE;

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
	
      case XK_T:
      case XK_t:      Toggle_tip_plotting(); break;
	
      case XK_U:
      case XK_u:      View_u_field(); break;
	
      case XK_V:
      case XK_v:      View_v_field(); break;
	
      case XK_N:
      case XK_n:      View_no_field(); break;

      case XK_S:
      case XK_s:      Save_image(); break;

      case XK_Right:  Mover(1,0);  break;    /* move +x */
      case XK_Left:   Mover(-1,0); break;    /* move -x */
      case XK_Up:     Mover(0,1);  break;    /* move +y */
      case XK_Down:   Mover(0,-1); break;    /* move -y */

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

    } /* switch (theEvent.type) */

  } /* while (XPending(theDisplay) || (ezmode != MODE_SIMULATING)) */

  /* restore write_tip */
  write_tip = write_tip_save;

  return(0);
}
/* ========================================================================= */

static void InitX (int winx, int winy, int width, int height)
{
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

  /* DOUBLE is the important one.  Perhaps need to add error checking when
   * DOUBLE_BUFFER not available.  In the aux library the first entry in list
   * was GLX_LEVEL, 0, */

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
   * to Name, which unless you change it (in ezgraph.h), it will be
   * EZ-Spiral. */

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
}
/* ========================================================================= */

static Bool WaitForNotify(Display *d, XEvent *e, char *arg) 
{
  /*  As seen in the Porting Guide. */
  return (e->type == MapNotify) && (e->xmap.window == (Window)arg);
}
/* ========================================================================= */

void QuitX (void)
{
  if(GRAPHICS)
    XCloseDisplay(theDisplay);
  return;
}
/* ========================================================================= */

static void Save_image (void)
{
  /* Save to a file an rgb image of what is currently on the screen. Usable
     only on SGI. */
#if 0  
  static num = 1;
  static char cmd[40];

  glFlush();
  sprintf(cmd, "scrsave snap%d.rgb %d %d %d %d", 
	  num++, WINX+8, WINX+WINSIZE-8, 1024-WINY-40, 1024-(WINY+WINSIZE));
  system(cmd);
#endif
}
/* ========================================================================= */


