/* ------------------------------------------------------------------------- */
/* ezmarching.h -- Macros for EZMARCHING
 *
 * Copyright (C) 1996, 1997, 1998 Dwight Barkley and Matthew Dowle
 *
 * RCS Information
 * ---------------------------
 * $Revision: 1.1.1.1 $
 * $Date: 2003/07/05 15:08:18 $
 * ------------------------------------------------------------------------- */

#ifndef _EZMARCHING_
#define _EZMARCHING_


#define U_CONT 0.5                        /* Iso-surface (contour) values */
#define V_CONT (a_param*U_CONT-b_param)   /* Set according to your needs  */

#define FILAMENT_R   1.0    /* R, */
#define FILAMENT_G   1.0    /* G, */
#define FILAMENT_B   1.0    /* B values for filament color: 0 <= R,G,B <= 1 */
#define FILAMENT_WT  3.0    /* filament line width */

#define BBOX_R       1.0    /* R, */
#define BBOX_G       0.0    /* G, */
#define BBOX_B       0.0    /* B values for bounding box: 0 <= R,G,B <= 1 */
#define BBOX_WT      2.0    /* bounding box line width */


/* ------------------------------------------------------------------------- 
 * All floating point numbers in ezmarching are of type GLReal. Defined here
 * to be GLfloat. If the precision of the numerical data is double then
 * implicit conversion will take place when calculating the contour intersect
 * and finding the cube index. You may wish to experiment and try setting
 * this to double or to Real.  The functions glVertex* and glColor* functions
 * have many forms so I use macros to make changes easier. 
 *
 * Note that we often need to define points (and vectors) in 3 space
 * dimensions. These are used both in calculations and as data to pass to
 * OpenGL for rendering.  All such variables are defined as arrays of size
 * 3. In cases where we need arrays of coordinates, the index for each
 * dimension should be the last one. This means that the X, Y and Z
 * components are contiguous in memory and enables us to use glVertex3fv
 * instead of glVertex3f. 
 * ------------------------------------------------------------------------- */

typedef GLfloat       GLReal;

#define GLVERTEX3V(v)    glVertex3fv(v)
#define GLVERTEX3(x,y,z) glVertex3f((x),(y),(z))
#define GLCOLOR3(r,g,b)  glColor3f((r),(g),(b))

/* ------------------------------------------------------------------------- 
 *
 *            Probably you do not want to change anything below              
 *
 * ------------------------------------------------------------------------- */

typedef unsigned char CubeEdge;       /* Number between 0 and 12. 0 means "no
				       * edge". Large tables of CubeEdge are
				       * defined so we want it as small as
				       * possible. */
typedef unsigned int  CubeVertex;     /* Numer between 1 and 8. */
typedef unsigned int  CubeIndex;      /* Number between 0 and 255. Could use
				       * unsigned char, but storage is not an
				       * issue and the machine is probably
				       * better at multiplying ints. Some
				       * machines are better at multiplying
				       * unsigned ints than signed ints. */
typedef unsigned int  Axis;           /* Either X, Y or Z. */


#define MAX_TRIANGLE_LIST_LENGTH  16  /* Maximum triangles in marching cube is
				       * 5 (each with 3 edges) plus 1 for the
				       * end marker. */
#define MAX_EDGE_LIST_LENGTH      13  /* As above but without
				       * repetitions. Indexes 90 and 165 (4
				       * non touching triangles) have an
				       * intersect on each of the 12
				       * edges. Plus one for the end
				       * marker. */
#define NUM_GENERATORS            21  /* Number of basic marching cubes cases
				       * which can be used to build the rest
				       * of the table. 1987 paper says 15, but
				       * with 1995 modification this goes up
				       * to 21 to deal with complimentary
				       * cases. */
#define END                        0  /* End of edge list marker. */

#define EQUAL_ZERO(n)  ((n) == 0.0)   /* In this code, this should be
				       * ok because we always clamp
				       * values afterwards and we're
				       * not too * concerned about
				       * underflow and overflow. */

#define OFF       0                   /* Draw_modes */ 
#define LINES     1
#define TRIANGLES 2

#define X 0                           /* Indices for vector components */
#define Y 1
#define Z 2

#define A 0                           /* Params for implicit form of plane. */
#define B 1
#define C 2
#define D 3


/* Vector operations used exclusively by the functions filament_in_cube() and
 * find_two_pts () which finds intersections of u and v triangles. */

#define SUB(ans,v1,v2) \
  (ans)[X] = (v1)[X] - (v2)[X]; \
  (ans)[Y] = (v1)[Y] - (v2)[Y]; \
  (ans)[Z] = (v1)[Z] - (v2)[Z]

#define DOT(v1, v2) ((v1)[X]*(v2)[X] + (v1)[Y]*(v2)[Y] + (v1)[Z]*(v2)[Z])

#define POINT_ALONG_LINE(ans, start, length, direction) \
  (ans)[X] = (start)[X] + (length)*(direction)[X]; \
  (ans)[Y] = (start)[Y] + (length)*(direction)[Y]; \
  (ans)[Z] = (start)[Z] + (length)*(direction)[Z]

#endif  /* _EZMARCHING_ */
