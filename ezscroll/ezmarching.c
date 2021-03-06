/* ------------------------------------------------------------------------- */
/* ezmarching.c -- Efficient implementation of the Marching Cubes Algorithm
 * first published by Bill Lorensen (1987).
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
#include <string.h>
#include <math.h>
#ifndef _GLONLY
#include <GL/gl.h>
#include <GL/glx.h>
#else /* _GLONLY */
#ifdef _MACOSX
#include <GLUT/glut.h>
#else /* !_MACOSX */
#include <GL/glut.h>
#endif /* _MACOSX */
#endif /* _GLONLY */

#include "ezscroll.h"    
#include "ezgraph3d.h"
#include "ezmarching.h"

#define Bool unsigned char

/* -------------------------------------------------------------------------
 *
 * This file contains all graphics functions that compute the iso-surfaces
 * and filaments, and render these.
 *
 * The important things to know about this file are:
 * 
 * (1) All functions containing OpenGL calls start with Draw_.  These
 * functions and Marching_cubes() are probably the only ones you would
 * need to change to affect the look of the graphics (contour colors etc.)
 *
 * (2) The U and V contours values are defined in ezmarching.h.  The filament
 * is defined as the intersection of these iso-surfaces.
 *
 * (3) There are three states the draw_mode can be in:
 *   OFF:       glEnd() has been called and nothing being passed to OpenGL.
 *   LINES:     glBegin(GL_LINES) has been called and points on line 
 *              segments are being passed.
 *   TRIANGLES: glBegin(TRIANGLES) has been called and triangle vertices 
 *              are being passed.
 *
 * (4) The last approx 1/4 of the code is devoted to initialization,
 * i.e. setting up the lookup table.  You shouldn't touch this.
 *
 * ------------------------------------------------------------------------- */


/* 
 * Global variables for this file only 
 * ----------------------------------- */

static CubeEdge        triangle_table[256][MAX_TRIANGLE_LIST_LENGTH];
                           /* triangle_table is the marching cubes lookup
			    * table containing triples of edges which define
			    * each triangle.  */

static CubeEdge        edge_table[256][MAX_EDGE_LIST_LENGTH];
                           /* Same as triangle_table but without repetitions
			    * of edges. */

static CubeEdge        *triangle_list[2];
                           /* triangle_list[U_FIELD] and
			    * triangle_list[V_FIELD] are used when filament
			    * plotting to find common lines of the
			    * triangles. */

static GLReal          vertex[2][13][3];
                            /* Triangle vertices in real world space for each
			     * edge in each volume. When just surface drawing
			     * we only use the U or V part of the array. When
			     * filament drawing we use both. */

static GLReal          contour[2];
                            /* The U and V values of the iso-surfaces. */

static GLReal          marching_h; 
                            /* Size of marching cube in graphics coordinates */

static GLReal 	       convert_to_phy;
                            /* Convert from graphics to physical coordinates */

static GLReal          pos[3];
                            /* Position of the 3rd vertex of the current
			     * marching cube in graphics coordinates. */

static Axis            axis[13]  = {0,Z,Y,Z,Y,Z,Y,Z,Y,X,X,X,X}; 
                            /* Given a cube edge, axis tells you which
			     * coordinate axis the edge is on. */

static unsigned int    ii,jj,kk;
                            /* Position of the 3rd vertex of the current
			     * marching cube on the simulation grid. */

static unsigned int    inc; 
                           /* The length of the marching (graphics) cube in
			    * terms of the simulation grid. The marching cube
			    * is defined by (ii,jj,kk) and
			    * (ii-inc,jj-inc,kk-inc).  Must be >= 1. */

static int             draw_mode=OFF;
                            /* draw_mode can be in one of three states. start
			     * with it off */

static Bool            filament_only; 
                            /* compute filament only */

static Bool            show_field;
                            /* draw and iso-surface on screen */

/* 
 * Private functions 
 * ----------------- */

/* OpenGL specific drawing functions */
static void       Draw_triangles          (int field, Real *plot_length);
static void       Draw_filament           (int clipping, GLReal point[2][3]);
static void       Set_draw_mode_lines     (int clipping, GLReal lwidth);
static void       Set_draw_mode_triangles (int clipping);
static void       Set_draw_mode_off       (void);
static void       Draw_bounding_box       (int clipping, Real *plot_length);

/* Functions for computing iso-surfaces and filaments */
static void       Compute_isoSurface      (int field, int show_filament, 
					   int clipping, Real *plot_length);
static void       Filament_in_cube        (int show_filament, int clipping); 
static void       Find_two_pts            (GLReal point[2][3], 
					   GLReal point_r[4], 
					   GLReal p[3], GLReal dir[3]);
static void       Set_relative_edge_pos   (int inc);
static GLReal     Contour_intersect       (int field, CubeEdge edge);

/* Functions used in initialization */
static CubeIndex  Rotate_edge_list        (CubeIndex index, int direction);
static CubeIndex  Rotate_edge_list_x      (CubeIndex index);
static CubeIndex  Rotate_edge_list_y      (CubeIndex index);
static void       Rotate_diagonal         (CubeIndex index);
static void       Do_all_orientations     (CubeIndex index);
static void       Generate_edge_table     (void);
static void       Print_list              (CubeEdge *p);

/* ========================================================================= */

void Marching_cubes (unsigned int resolution, int field, int show_filament, 
		     int clipping, Real *plot_length)
{
  Draw_bounding_box(clipping, plot_length);

  if (field == NO_FIELD && !show_filament && !write_filament) return;
  inc = resolution;
  marching_h = inc*plot_length[0]/(NX-1);  
  convert_to_phy = grid_h*(inc/marching_h);
  Set_relative_edge_pos(inc);
  contour[0] = U_CONT;
  contour[1] = V_CONT;

  /* If field == NO_FIELD then just the filament is to be found.  In
   *   this case can set and leave the draw mode as lines.  Set field
   *   to U (to find filament we must first find an iso-surface) and at
   *   end re-set to NO_FIELD.  
   * Otherwise we will be drawing triangles. */

  if (field == NO_FIELD) {           
    filament_only = TRUE;
    show_field = FALSE;
    Set_draw_mode_lines(clipping, FILAMENT_WT);
    field = U_FIELD;  
  }
  else {
    filament_only = FALSE;
    show_field = TRUE;
    Set_draw_mode_triangles(clipping);
  }

  /* Compute iso-surface and/or filament */
  Compute_isoSurface(field, show_filament, clipping, plot_length);

  /* Finish up */
  Set_draw_mode_off();
  if (filament_only) field = NO_FIELD;
}
/* ========================================================================= */

static void Draw_triangles (int field, Real *plot_length)
{
  GLReal red, green, blue;
  int n;

  /* -----------------------------------------------------------------
   *                    Set the iso-surface color.
   * Can also choose not to render some triangles.  Examples are given
   * Note: I only color at the marching cube level, i.e. all triangles
   * within a given marching cube get the same color.  One could color
   * at the individual triangle level (inside the while loop).
   * ----------------------------------------------------------------- */

#if 0   /* Do not draw triangles if v(ii,jj,kk) > V_CONT */
  if( v(ii,jj,kk) > V_CONT) return;
#endif

#if 1   /* Color according to position in space */
  red   = pos[Y]/plot_length[1]; 
  green = pos[Z]/plot_length[2]; 
  blue  = pos[X]/plot_length[0];
#endif

#if 0   /* Color (yellow-green) according to other field (1-field)*/
  red   = 1.- Fields(1-field,ii,jj,kk);
  green = 1.;
  blue  = 0.;
#endif

#if 0   /* Color (red-blue) according to other field */
  red   = Fields(1-field,ii,jj,kk);
  green = 0.;
  blue  = 1.- Fields(1-field,ii,jj,kk);
#endif

#if 0   /* Vary the intensity (when coloring according to other field) */
  {float intensity = pow( (pow(pos[Y]/plot_length[1],2.)
			   + pow(pos[Z]/plot_length[2],2.)
			   + pow(pos[X]/plot_length[0],2.))/3., 0.25);
  red   *= intensity;
  green *= intensity;
  blue  *= intensity;
  }
#endif

  GLCOLOR3(red, green, blue);

  /* Send OpenGL the positions of each triangle vertex.  Every group of 3
   * vertices is interpreted as defining one triangle. The triangle table is
   * organized so that all we need do now is pass all the vertices in the
   * list to OpenGL. */

  n = 0;
  while (triangle_list[field][n] != END) { 
    GLVERTEX3V(vertex[field][triangle_list[field][n]]);
    n++;
  }
}
/* ========================================================================= */

static void Draw_filament (int clipping, GLReal point[2][3])
{
  Set_draw_mode_lines(clipping, FILAMENT_WT);
  GLCOLOR3(FILAMENT_R, FILAMENT_G, FILAMENT_B);
  GLVERTEX3V(point[0]);
  GLVERTEX3V(point[1]);
}
/* ========================================================================= */

static void Set_draw_mode_triangles (int clipping)
{
  if(draw_mode==TRIANGLES) return;

  if(draw_mode!=OFF) glEnd();

  if(clipping) glEnable(GL_CLIP_PLANE0); 
  glBegin(GL_TRIANGLES);
  draw_mode=TRIANGLES;
}
/* ========================================================================= */

static void Set_draw_mode_lines (int clipping, GLReal lwidth)
{
  if(draw_mode==LINES) return;

  if(draw_mode!=OFF) glEnd();
  if(clipping) glDisable(GL_CLIP_PLANE0);  /* we never clip lines */
  glLineWidth(lwidth);
  glBegin(GL_LINES);
  draw_mode=LINES;
}
/* ========================================================================= */

static void Set_draw_mode_off (void)
{
  glEnd();
  draw_mode=OFF;
}
/* ========================================================================= */

void Draw_bounding_box (int clipping, Real *plot_length)
{
  Set_draw_mode_off();
  Set_draw_mode_lines(clipping, BBOX_WT);

  GLCOLOR3(BBOX_R, BBOX_G, BBOX_B);

  GLVERTEX3(0.,0.,0.);  
  GLVERTEX3(plot_length[0],0.,0.);
  GLVERTEX3(plot_length[0],0.,0.);  
  GLVERTEX3(plot_length[0],plot_length[1],0.);
  GLVERTEX3(plot_length[0],plot_length[1],0.);  
  GLVERTEX3(0.,plot_length[1],0.);
  GLVERTEX3(0.,plot_length[1],0.);  
  GLVERTEX3(0.,0.,0.);

  GLVERTEX3(0.,0.,0.);  
  GLVERTEX3(0.,0.,plot_length[2]);
  GLVERTEX3(plot_length[0],0.,0.);  
  GLVERTEX3(plot_length[0],0.,plot_length[2]);
  GLVERTEX3(plot_length[0],plot_length[1],0.);  
  GLVERTEX3(plot_length[0],plot_length[1],plot_length[2]);
  GLVERTEX3(0.,plot_length[1],0.);  
  GLVERTEX3(0.,plot_length[1],plot_length[2]);

  GLVERTEX3(0.,0.,plot_length[2]);  
  GLVERTEX3(plot_length[0],0.,plot_length[2]);
  GLVERTEX3(plot_length[0],0.,plot_length[2]);  
  GLVERTEX3(plot_length[0],plot_length[1],plot_length[2]);
  GLVERTEX3(plot_length[0],plot_length[1],plot_length[2]);  
  GLVERTEX3(0.,plot_length[1],plot_length[2]);
  GLVERTEX3(0.,plot_length[1],plot_length[2]);  
  GLVERTEX3(0.,0.,plot_length[2]);

  Set_draw_mode_off();
}
/* ========================================================================= */

static void Compute_isoSurface (int field, int show_filament, int clipping, 
				Real *plot_length)
{
  static CubeIndex  index, other_index;
  static CubeEdge   edge;
  static int        n;

  pos[X] = 0.0;
  for (ii=1+inc; ii<=NX; ii+=inc) {
    pos[X] += marching_h;

    /* Each triangle vertex has only one linearly interpolated component
     * (coordinate). The other 2 are vertices lie on a cube edge and are
     * known.  So for this Y-Z plane, we already know the X component of all
     * the edges not parallel to the X axis. The vertices on the edges
     * parallel to the X axis will be linearly interpolated as necessary
     * later. */

    vertex[field][1][X] = 
    vertex[field][2][X] = 
    vertex[field][3][X] = 
    vertex[field][4][X] = pos[X];
    vertex[field][5][X] = 
    vertex[field][6][X] = 
    vertex[field][7][X] = 
    vertex[field][8][X] = pos[X] - marching_h;

    pos[Y] = 0.0;
    for (jj=1+inc; jj<=NY; jj+=inc) {
      pos[Y] += marching_h;
    
      /* Similar to above, we can set the components of each edge not on the
	 Y axis. */

      vertex[field][1][Y]  = 
      vertex[field][5][Y]  = 
      vertex[field][9][Y]  = 
      vertex[field][10][Y] = pos[Y] - marching_h;
      vertex[field][3][Y]  = 
      vertex[field][7][Y]  = 
      vertex[field][11][Y] = 
      vertex[field][12][Y] = pos[Y];

      pos[Z] = 0.0;
      for (kk=1+inc; kk<=NZ; kk+=inc) {
	pos[Z] += marching_h;

	/* Calculate the index for the current cube for the current field.
	 * Recall, the cube is defined by (i,jj,kk) to (i-inc,jj-inc,kk-inc).
	 * The index considers the cube vertex (i,jj,kk) to be bit 3 of the
	 * index. */

	index =
	  ((Fields(field,ii    ,jj-inc,kk-inc) >= contour[field])     ) |
	  ((Fields(field,ii    ,jj-inc,kk)     >= contour[field]) << 1) |
	  ((Fields(field,ii    ,jj    ,kk)     >= contour[field]) << 2) |
	  ((Fields(field,ii    ,jj    ,kk-inc) >= contour[field]) << 3) |
	  ((Fields(field,ii-inc,jj-inc,kk-inc) >= contour[field]) << 4) |
	  ((Fields(field,ii-inc,jj-inc,kk)     >= contour[field]) << 5) |	      
	  ((Fields(field,ii-inc,jj    ,kk)     >= contour[field]) << 6) |
	  ((Fields(field,ii-inc,jj    ,kk-inc) >= contour[field]) << 7);

	/* If the index is non-trivial then the contour passes through this
	 * cube. Otherwise we are finished. */

	if ((index != 0) && (index != 255)) {
	
	  /* Calculate the remaining components of any triangle vertices
	   * known without interpolation.  We did the X and Y components
	   * earlier. */

	  vertex[field][2][Z]  = 
	  vertex[field][6][Z]  = 
	  vertex[field][10][Z] = 
	  vertex[field][12][Z] = pos[Z];
	  vertex[field][4][Z]  = 
	  vertex[field][8][Z]  = 
	  vertex[field][9][Z]  = 
	  vertex[field][11][Z] = pos[Z] - marching_h;

	  /* Linearly interpolate along the necessary cube edges to finish up
	   * the vertices */

	  n = 0;
	  while ((edge = edge_table[index][n]) != END) {
	    vertex[field][edge][axis[edge]] = Contour_intersect(field,edge);
	    n++;
	  }
	  triangle_list[field] = triangle_table[index];

	  /* All the triangles have been found. Draw them if necessary */

	  if (show_field) Draw_triangles (field, plot_length);

	  if (show_filament || write_filament) {   /* If filament is needed
						    * compute  it, otherwise 
						    * we are finished */

	    /* Calculate index for other field */

	    other_index =
	    ((Fields(1-field,ii    ,jj-inc,kk-inc) >= contour[1-field])     ) |
	    ((Fields(1-field,ii    ,jj-inc,kk)     >= contour[1-field]) << 1) |
	    ((Fields(1-field,ii    ,jj    ,kk)     >= contour[1-field]) << 2) |
	    ((Fields(1-field,ii    ,jj    ,kk-inc) >= contour[1-field]) << 3) |
            ((Fields(1-field,ii-inc,jj-inc,kk-inc) >= contour[1-field]) << 4) |
	    ((Fields(1-field,ii-inc,jj-inc,kk)     >= contour[1-field]) << 5) |
	    ((Fields(1-field,ii-inc,jj    ,kk)     >= contour[1-field]) << 6) |
	    ((Fields(1-field,ii-inc,jj    ,kk-inc) >= contour[1-field]) << 7);

	    if ((other_index != 0) && (other_index != 255)) {

	      /* Both u and v contours pass through this cube, so there is
	       * potential for contour intersection (i.e. the filament) */

	      triangle_list[1-field] = triangle_table[other_index];

	      /* Copy over the existing vertices. Each of which only one
		 component may differ (if an intersect exists). */

	      for (n=1; n<=12; n++) {
		vertex[1-field][n][X] = vertex[field][n][X];
		vertex[1-field][n][Y] = vertex[field][n][Y];
		vertex[1-field][n][Z] = vertex[field][n][Z];
	      }

	      /* Do the interpolation */

	      n = 0;
	      while ((edge = edge_table[other_index][n]) != END) {
		vertex[1-field][edge][axis[edge]] =
		  Contour_intersect(1-field,edge);
		n++;
	      }

	      /* Now we have all the U and V triangles for this cube so we
	       * can look for the filament. Cube edges that do not contain a
	       * triangle vertex will contain junk values. */

	      /* You may want to try organizing things differently when just
	       * filament is required. At the moment, the Y and Z components
	       * of the vertices are being set too early. You only need set
	       * them before the following call. However, they are only
	       * assignments: the X is ok because it would be unusual not to
	       * get a filament through some part of each Y-Z plane; the Z is
	       * only when the U is non-trivial. */

	      Filament_in_cube(show_filament, clipping);
	      
	      /* If plotting iso-surfaces, return the state to GL_TRIANGLES */

	      if(show_field) Set_draw_mode_triangles(clipping);
	    }
	  }
	}
      }
    }
  }
}
/* ========================================================================= */

static void Filament_in_cube (int show_filament, int clipping)
{
  /* At the current cube (ii,jj,kk) both U and V have a non-trivial index.
   * This function finds any triangle intersections (i.e. filament) in this
   * cube.
   *
   * We look at each pair of triangles and find their common line if it
   * exists. The filament thus consists of many small lines.  Experience has
   * shown that usually there are usually only 2 (or maybe 3) triangles in
   * each cube and for this small number of triangles the algorithm should be
   * efficient. Other methods were considered such as following the filament
   * through the cube but then issues such as what do you when there is more
   * than one filament in one cube complicate such a solution. */

  static unsigned int  field;
  static unsigned int  t_num[2];          /* Number of triangle (0-4) */
  static unsigned int  t_pos[2];          /* Position in triangle list. */
  static CubeEdge      t_cube_edge[2][3]; /* The cube edges of the current U
					   * and V triangle. */
  static GLReal        t_edge_vect[2][5][3][3];
                                          /* All the triangle edge vectors. */
  static GLReal        *te0, *te1, *te;   /* Convenience pointers. */
  static GLReal        plane[2][5][4];    /* Plane coefficients for each
					   * triangle. */
  static GLReal        *u_plane, *v_plane;/* Convenience pointers. */
  static GLReal        p_t0[3], p_t1[3];  /* Vector going from p (the reference
					   * point on the common line) to
					   * the triangle vertex. */
  static GLReal        point[2][3];       /* The line segment to be drawn. */
  static GLReal        den,dc,db,ad;      /* Arithmetic storage. */
  static unsigned int  tri_edge;          /* Loop variable. */
  static GLReal        r,s;               /* Arithmetic storage. */
  static GLReal        p[3], dir[3]; 
  static GLReal        point_r[4];        /* Used to find the
					   * filament. Defines the intersects
					   * of the common plane line with
					   * each triangle edge. */
  static unsigned int  points_found;      /* Intersection points found so far
					   * between the common plane line and
					   * triangle edges. */
  static Axis          dim1,dim2;         /* Dimensions to perform line
					   * intersection in. */
  static Axis          next_axis[3] = {Y,Z,X};
                                          /* Convenient array to cycle axes. */
  static unsigned int  count;             /* How many times you've cycles the
					   * axes. */
  
  /* For each U triangle (then for each V triangle) determine the
   * coefficients of the implicit form of the plane through each
   * triangle. That is, find a,b,c,d in a*x + b*y + c*z + d = 0. This is done
   * _once only_ for each triangle. */

  field = U_FIELD;
  do {             /* for each field */ 

    t_num[U_FIELD] = 0;  /* It doesn't matter in this loop, but use U_FIELD */
    t_pos[U_FIELD] = 0;

    while( (t_cube_edge[U_FIELD][0] = triangle_list[field][t_pos[U_FIELD]]) 
	   != END) {

      /* t0, t1 and t2 are labels for the 3 triangle vertices.  
       * te0, te1, te2 are labels for the vectors along each triangle edge. */

      t_cube_edge[U_FIELD][1] = triangle_list[field][t_pos[U_FIELD]+1];
      t_cube_edge[U_FIELD][2] = triangle_list[field][t_pos[U_FIELD]+2];

      /* Calculate vectors along 2 of the edges of the triangle. Remember them
       * for later. */

      SUB(t_edge_vect[field][t_num[U_FIELD]][0],
	  vertex[field][t_cube_edge[U_FIELD][1]], 
	  vertex[field][t_cube_edge[U_FIELD][0]]);

      SUB(t_edge_vect[field][t_num[U_FIELD]][1],
	  vertex[field][t_cube_edge[U_FIELD][2]], 
	  vertex[field][t_cube_edge[U_FIELD][0]]);

      SUB(t_edge_vect[field][t_num[U_FIELD]][2],
	  vertex[field][t_cube_edge[U_FIELD][2]], 
	  vertex[field][t_cube_edge[U_FIELD][1]]);

      te0 = t_edge_vect[field][t_num[U_FIELD]][0];   /* te0 = t1 - t0 */
      te1 = t_edge_vect[field][t_num[U_FIELD]][2];   /* te1 = t2 - t0 */
                                               /* te2 = t2 - t1  which we may
						* need later */

      plane[field][t_num[U_FIELD]][A] = te0[Y]*te1[Z] - te0[Z]*te1[Y];
      plane[field][t_num[U_FIELD]][B] = te0[Z]*te1[X] - te0[X]*te1[Z];
      plane[field][t_num[U_FIELD]][C] = te0[X]*te1[Y] - te0[Y]*te1[X];    
      plane[field][t_num[U_FIELD]][D] =
	-(vertex[field][t_cube_edge[U_FIELD][1]][X]
                                             *plane[field][t_num[U_FIELD]][A] +
	  vertex[field][t_cube_edge[U_FIELD][1]][Y]
                                             *plane[field][t_num[U_FIELD]][B] +
	  vertex[field][t_cube_edge[U_FIELD][1]][Z]
                                             *plane[field][t_num[U_FIELD]][C]);

      t_num[U_FIELD] += 1;        
      t_pos[U_FIELD] += 3;
    }
    field++;
  } while (field == V_FIELD);

  /* Now look at each pair of triangles from the U and V cubes. */

  t_num[U_FIELD] = 0;           /* Triangle number (0-4) */
  t_pos[U_FIELD] = 0;

  while ( (t_cube_edge[U_FIELD][0] = triangle_list[U_FIELD][t_pos[U_FIELD]]) 
	  != END) {

    /* For each triangle in the u cube */

    t_cube_edge[U_FIELD][1] = triangle_list[U_FIELD][t_pos[U_FIELD]+1];
    t_cube_edge[U_FIELD][2] = triangle_list[U_FIELD][t_pos[U_FIELD]+2];

    t_num[V_FIELD] = 0;
    t_pos[V_FIELD] = 0;

    while ( (t_cube_edge[V_FIELD][0] = triangle_list[V_FIELD][t_pos[V_FIELD]]) 
	    != END) {
      /* For each triangle in the v cube */

      t_cube_edge[V_FIELD][1] = triangle_list[V_FIELD][t_pos[V_FIELD]+1];
      t_cube_edge[V_FIELD][2] = triangle_list[V_FIELD][t_pos[V_FIELD]+2];
      
      /* -----------------------------------------------------
       *  Start of code that find the common line of 2 planes. 
       * ----------------------------------------------------- */

      /* Find the common line :   x[] = p[] + t*dir[] */

      u_plane = plane[U_FIELD][t_num[U_FIELD]];
      v_plane = plane[V_FIELD][t_num[V_FIELD]];

      dir[X] = u_plane[B]*v_plane[C] - v_plane[B]*u_plane[C];
      dir[Y] = u_plane[C]*v_plane[A] - v_plane[C]*u_plane[A];
      dir[Z] = u_plane[A]*v_plane[B] - v_plane[A]*u_plane[B];

      den = DOT(dir,dir);

      if (EQUAL_ZERO(den)) {
	/* Planes are parallel so no intersect. 
	 * Overlapping co-planar triangles will be discarded here. */
      }
      else {

	dc = u_plane[D]*v_plane[C] - u_plane[C]*v_plane[D];
	db = u_plane[D]*v_plane[B] - u_plane[B]*v_plane[D];
	ad = u_plane[A]*v_plane[D] - v_plane[A]*u_plane[D];
	
	p[X] =  (dir[Y]*dc - dir[Z]*db) / den;
	p[Y] = -(dir[X]*dc + dir[Z]*ad) / den;
	p[Z] =  (dir[X]*db + dir[Y]*ad) / den;
	
	/* In fact p is the closest point on the line to the origin. But we
	 * do not use that information here. */
	
	/* --------------------------------------------------------- 
	 * Start of code that finds subset of line in both triangles 
	 * --------------------------------------------------------- */
	
	/* We find where the common line cuts the triangle edges. This will
	 * give us either 0, 2 or 4 points for sure. For the triangles to
	 * intersect, we must have 2 points from the U edges and 2 points
	 * from the V edges. We can stop looking as soon as these conditions
	 * are not met.
	 *
	 * By construction, the common line lies in the same plane as both
	 * triangles. Thus our arithmetic need only be in 2 dimension. We must
	 * be careful in the case where the triangle is normal to an
	 * axis. This will happen rarely under FPA, but is most likely at the
	 * initial condition where scalar values are set by the user.  You
	 * might want to see if there is a better way of handling such
	 * cases. I don't think the method I use here is the best. */

	points_found = 0;
	field = U_FIELD;

	do {  /* for each field */ 

	  SUB(p_t0, vertex[field][t_cube_edge[field][0]], p);

	  tri_edge = 0;
	  do /* for first 2 edges */ {

	    te = t_edge_vect[field][t_num[field]][tri_edge];
	    /* "te" <=> "triangle edge" */
				      
	    den = 0.0;
	    dim1 = X; dim2 = Y;
	    count = 0;
	    while (EQUAL_ZERO(den) && count < 3) {
	      dim1 = next_axis[dim1];
	      dim2 = next_axis[dim2];
	      den = te[dim2]*dir[dim1] - te[dim1]*dir[dim2];
	      count ++;
	    }
	    
	    if (EQUAL_ZERO(den)) {
	      /* Common line and triangle edge are parallel. No intersect. */
	      /* This happens quite a lot at the start of the simulation
	       * because scalar values are still close to their initial
	       * values. Holes can be seen. Later on, this doesn't happen
	       * at all. */
	    }
	    else {
	      s = (dir[dim2]*p_t0[dim1] - dir[dim1]*p_t0[dim2]) / den;

	      if ((0.0 <= s) && (s <= 1.0)) {
		/* Intersection is on the triangle edge. If its actually on
		 * a vertex then it could still form the end of a filament
		 * segment (unlikely though). */

		r = (te[dim2]*p_t0[dim1] - te[dim1]*p_t0[dim2]) / den;
		/* This r is used later to give us an ordering on the points
		 * found. */

		point_r[points_found] = r;
		points_found++;
	      }
	    }
	    tri_edge += 1;
	  } while (tri_edge == 1);

	  /* We've done edges 0 and 1. If there have been no points found yet
	   * then the common line cannot pass through this triangle. If we've
	   * found 2 then we need to do no more and can look at the V
	   * triangle.  If we've found 1 then then the other points will be
	   * on the last edge. */

	  if ((points_found == 1) || (points_found == 3)) {
	  
	    /* This whole loop is repeated for the V field and is the reason
	     * this needs to be done for the 3rd edge of the V triangle. */

	    /* We know for sure that we're going to find another point on the
	       3rd edge. */

	    te = t_edge_vect[field][t_num[field]][2];

	    den = 0.0;
	    dim1 = X; dim2 = Y;
	    count = 0;
	    while (EQUAL_ZERO(den) && count < 3) {
	      dim1 = next_axis[dim1];
	      dim2 = next_axis[dim2];
	      den = te[dim2]*dir[dim1] - te[dim1]*dir[dim2];
	      count ++;
	    }

	    if (EQUAL_ZERO(den)) {
	      /* This should never happen. Test anyway. */
	    }
	    else {
	      p_t1[dim1] = vertex[field][t_cube_edge[field][1]][dim1]- p[dim1];
	      p_t1[dim2] = vertex[field][t_cube_edge[field][1]][dim2]- p[dim2];
	      /* We don't need the remaining dimension for the calculation. */
	    
	      s = (dir[dim2]*p_t1[dim1] - dir[dim1]*p_t1[dim2]) / den;
	      if ((0.0 <= s) && (s <= 1.0)) {
		/* This _should_ always happen. Test anyway. */
		r = (te[dim2]*p_t1[dim1] - te[dim1]*p_t1[dim2]) / den;
	    
		point_r[points_found] = r;
		points_found++;
	      }
	    }
	  }

	  /* Found all the points we are going to on the U triangle. Now
	   * repeat for the V triangle as long as we found exactly 2
	   * points. */

	  field += 1;
	} while ((field == V_FIELD) && (points_found == 2));

	if (points_found == 4) {
	  Find_two_pts(point, point_r, p, dir);

	  /* -----------------------------------------------------
	   * A piece of the filament has been found. Plot or write 
	   * it as necessary.  Write_filament_data() is defined in 
	   * ezscroll.c 
	   * ----------------------------------------------------- */

	  if(show_filament)  Draw_filament(clipping, point);
	  if(write_filament) Write_filament_data(convert_to_phy*point[0][0],
						 convert_to_phy*point[0][1],
						 convert_to_phy*point[0][2],
						 convert_to_phy*point[1][0],
						 convert_to_phy*point[1][1],
						 convert_to_phy*point[1][2]);
	}
      } /* end of dealing with common line */

      /* Try next V triangle. */
      t_num[V_FIELD] += 1;
      t_pos[V_FIELD] += 3;
    }
    
    /* Try next U triangle. */
    t_num[U_FIELD] += 1;
    t_pos[U_FIELD] += 3;
  }
}
/* ========================================================================= */

static void Find_two_pts (GLReal point[2][3], GLReal point_r[4], GLReal p[3],
			  GLReal dir[3])
{
  /* This function takes the 4 r's in point_r[4] which give the intersections
   * of the common plane line with the triangle edges. It then draws the
   * subset of the common line which lies within both triangles.
   *
   * It is assumed that the r's have been correctly stored in point_r[4]. The
   * first 2 entries will be the U intersections, the 2nd two will be the
   * V. They probably will not be in order. 
   *
   * There must be a quicker way to do this, surely? I've tried Batcher's
   * optimal shuffle but the method below uses less 'ifs' I think because
   * this is a special case. We only need the middle 2 points which should
   * simplify things. */

  static unsigned int u_min, v_min;
  static unsigned int second_position, third_position;
  static unsigned int higher_than_second, also_higher_than_second;

  /* Find order of the U pair */
  if (point_r[1] > point_r[0]) u_min = 0;
  else                         u_min = 1;

  /* Find order of the V pair */
  if (point_r[3] > point_r[2]) v_min = 0;
  else                         v_min = 1;

  /* Find minimum overall */
  if (point_r[v_min+2] > point_r[u_min]) {

    /* Lowest value is the lowest U. The next highest must be the lowest V for
     * there to be a common region. This will happen if the lowest V is lower
     * than the highest U. */

    second_position = v_min+2;
    higher_than_second = (1-u_min);
    also_higher_than_second = (1-v_min)+2;
  } 
  else {

    /* Lowest value is the lowest V. The next highest must be the lowest U for
     * there to be a common region. This will happen if the lowest U is lower
     * than the highest V. */

    second_position = u_min;
    higher_than_second = (1-v_min)+2;
    also_higher_than_second = (1-u_min);
  }

  if (point_r[second_position] < point_r[higher_than_second]) {

    /* There is a line but we don't know which is the third position. */

    if (point_r[higher_than_second] < point_r[also_higher_than_second]) {
      third_position = higher_than_second;
    }
    else {
      third_position = also_higher_than_second;
    }

    /* set the points */

    POINT_ALONG_LINE(point[0], p, point_r[second_position], dir);
    POINT_ALONG_LINE(point[1], p, point_r[third_position], dir);
  }
}
/* ========================================================================= */

/* Given a cube edge, relative_edge_pos tells you how to get from (ii,jj,kk)
 * to the end of the edge which is furthest along the edge's axis. The entry
 * for edge 1 tells you that you need to move down the Y axis from (ii,jj,kk)
 * and arrive at (ii,jj-1,kk). It also tells you that you do not need to move
 * along either the X or the Z axis. This information is used by
 * Contour_intersect() which calculates the intersection of the iso-surface
 * with any given cube edge. The fact that the cube vertex "is further along
 * the edges axis" is important because then you know that the intersect
 * always lies behind you. This is why there is always at least one '0' in
 * each entry in the table.
 *
 * Note that in fact the 1's are replaced by "inc" in Set_relative_edge_pos()
 * This is because the graphics resolution is variable so you might want to
 * move along an axis by more than one numerical cube length. Doing this
 * saves some multiplications in Contour_intersect(). */

static unsigned int relative_edge_pos[13][3] = {
  {0,0,0}, {0,1,0}, {0,0,0}, {0,0,0}, {0,0,1}, {1,1,0},
  {1,0,0}, {1,0,0}, {1,0,1}, {0,1,1}, {0,1,0}, {0,0,1}, {0,0,0} };

static void Set_relative_edge_pos (int inc)
{
  int p, q;
  for (p=1; p<=12; p++) {
    for (q=0; q<=2; q++) {
      if (relative_edge_pos[p][q] != 0) relative_edge_pos[p][q] = inc;
    }
  }
}
/* ========================================================================= */

static GLReal Contour_intersect (int field, CubeEdge edge)
{
  /* This function returns the position of the intersection of a contour with
   * the cube edge specified. It is assumed that the edge specified contains
   * a contour otherwise a divide by zero may occur. You can't go wrong as
   * long as you use the cube index to point to the edge table which will
   * tell you which edges have an intersect.
   *
   * The other components of the intersect can be determined by the calling
   * routine because they are the same as the position of the cube edge.
   *
   * The local variable 'field' overrides the global one. The local one tells
   * us which field to interpolate. The global one tells us which field we
   * are currently viewing (which is not useful here). */

  static unsigned int  begin [3],
                       end   [3];
  static GLReal        ratio;

  /* Set end to be the position of the node in the numerical volume which is
   * furthest along the axis on the specified edge specified. See declaration
   * of relative_edge_pos. See Marching_cubes(). */

  end[X] = ii-relative_edge_pos[edge][X];
  end[Y] = jj-relative_edge_pos[edge][Y];
  end[Z] = kk-relative_edge_pos[edge][Z];

  /* The other end of the edge differs only along its axis by inc. */

  begin[X] = end[X];
  begin[Y] = end[Y];
  begin[Z] = end[Z];
  begin[axis[edge]] -= inc;

  /* Calculate the ratio that the intersect occurs along the cube edge from
   * the end. */

  ratio = (contour[field]-Fields(field,end[X],end[Y],end[Z]))
    / ( Fields(field,begin[X],begin[Y],begin[Z])
       -Fields(field,end[X],end[Y],end[Z]) );

  return (pos[axis[edge]] - marching_h*ratio);
}
/* ========================================================================= */


/* ------------------------------------------------------------------------- 
 *     Here starts the initialization part of the code in which the lookup
 *                           tables are generated. 
 *		     
 * ------------------------------------------------------------------------- */

/* The main lookup table is the marching cubes lookup table called triangle
 * table. Each of the 8 vertices of a marching cube are assigned either a 0
 * or 1 depending on whether they are above or below the contour value. Given
 * this marching cubes index, it holds the instructions on how to draw the
 * contour through cube using at most 5 triangles.
 * 
 * There are 21 topologically distinct ways you can triangulate a cube. These
 * are listed below together with the index to which they correspond. By
 * rotating such cubes through all 24 orientations we can generate all the
 * entries in the lookup table.
 *
 * For indexes with 4 vertices high and 4 vertices low, rotating the cube
 * will eventually generate the entry for the complement of the original
 * index.  The triangle list so generated could well be topologically
 * different to the original triangle list. The 1987 paper used topologically
 * identical triangulations for complimentary cases. This has since found to
 * produce 'holes' in certain cases and so Montani, Scateni and Scopigno
 * (1994) devised 6 modified triangulation for complimentary cases. 3 of
 * these have 4 vertices high and 4 low and fortunately these are precisely
 * the cases which are topologically different when rotated to their
 * complimentary index.  The other 3 are new triangulations but do not have 4
 * vertices high and 4 vertices low so rotating these does not cause any
 * problems.
 * ------------------------------------------------------------------------- */


static CubeEdge table_generators[NUM_GENERATORS][MAX_TRIANGLE_LIST_LENGTH+1] =

/* The "+1" is for the first element - the index of the generator which is
 * used later to build the whole table. */

{

/* Unambiguous cases which have less than 4 vertices high. Rotating one of
 * these does not generate the complimentary case so we list them
 * explicitly. */

  {1,  1,4,9,END},                /*  case 1  */
  {254,1,4,9,END},                /*  case 1 compliment  */
  {3,  10,9,2,9,2,4,END},         /*  case 2  */
  {252,10,9,2,9,2,4,END},         /*  case 2 compliment  */
  {65, 1,4,9,7,12,6,END},         /*  case 4  */
  {190,1,4,9,7,12,6,END},         /*  case 4 compliment  */
  {50, 6,8,2,8,2,9,2,9,1,END},    /*  case 5  */
  {205,6,8,2,8,2,9,2,9,1,END},    /*  case 5 compliment  */

/* Unambiguous cases which have exactly 4 vertices high. The compliments will
 * be generated by the rotations. Note that the exact of order of the
 * triangle vertices for the complement will not be the same (because we
 * obtain them by rotational symmetry not complimentary symmetry) but the
 * triangle edges on each face are the same. */

  {51, 2,4,6,4,6,8,END},                 /*  case 8  */
  {78, 11,4,7,4,7,1,7,1,6,1,6,10,END},   /*  case 9  */
  {113,7,8,12,8,12,1,12,1,10,8,1,4,END}, /*  case 11 */
  {77, 2,1,6,1,6,11,6,11,7,1,11,9,END},  /*  case 14 */

/* Ambiguous cases with less than 4 vertices high and have different
 * compliments according to "A modified look-up table for implicit
 * disambiguation of Marching Cubes" by Montani, Scateni, Scopigno; Visual
 * Computer 1994 (10).  */

  {5,  1,4,9,3,2,12,END},                       /*  case 3   */
  {250,4,9,3,9,3,12,12,9,2,9,2,1,END},          /*  case 3 compliment  */
  {67, 10,9,2,9,2,4,7,12,6,END},                /*  case 6   */
  {188,10,9,6,9,6,4,6,4,7,4,7,12,12,4,2,END},   /*  case 6 compliment  */
  {74, 1,10,2,4,3,11,7,12,6,END},               /*  case 7   */
  {181,11,4,7,4,7,1,7,1,6,1,6,10,3,2,12,END},   /*  case 7 compliment  */

/* Ambiguous cases with exactly 4 vertices high. Compliments will be
 * generated by rotation which agree with the 1994 paper. And it is important
 * in these cases that the inverse of the index is topologically different.
 * */

  {105,11,9,3,9,3,1,7,5,12,5,12,10,END},        /*  case 10  */
  {58, 6,8,2,8,2,9,2,9,1,11,3,4,END},           /*  case 12  */
  {90, 4,1,9,11,7,8,2,3,12,5,6,10,END},         /*  case 13  */
};

/* Side note
 *
 * We have experimented using GL_TRIANGLE_STRIP in order to simplify the list
 * of triangles and reduce the number of vertices needed to pass to OpenGL
 * (and thus reduce the number of transformations required). However, in
 * order to do this, you need more glBegin and glEnds in order to keep
 * changing primitives. We believe that it is easier all round to provide
 * OpenGL with just GL_TRIANGLES. It is also very convenient because it also
 * makes finding surface intersection easier; everything is a triangle!  */

/* ------------------------------------------------------------------------- */

/* To generate the look up table, we simply rotate each of the above
 * generators through all 24 orientations. Under a rotation about the X axis,
 * x_mod_edge tells you where each edge is mapped. x_mod_vertex tells you
 * where each cube vertex is mapped. Similarly for a rotation about the Y
 * axis. */

static CubeEdge   x_mod_edge[13]  = {0,3,12,7,11,1,10,5,9,4,2,8,6};
static CubeVertex x_mod_vertex[9] = {0,8,4,64,128,1,2,32,16};

static CubeEdge   y_mod_edge[13]  = {0,9,4,11,8,10,2,12,6,5,1,7,3};
static CubeVertex y_mod_vertex[9] = {0,16,1,8,128,32,2,4,64}; 

/* ------------------------------------------------------------------------- */

void Marching_ini (void)
{
  CubeIndex  index;
  int        gen;    

  /*  Initialize the triangle table to be empty  */
  for (index=0; index<=255; index++) {
    triangle_table[index][0] = END;
  }

  /* Copy all the basic 14 recipes into the triangle table and do all 24
   * orientations for each one. This is extremely inefficient. However, the
   * code is concise and easier to debug. Most entries are generated in more
   * than one way (and an error is generated if any inconsistencies
   * arise). Thus we can be confident our table is correct. This is only done
   * once, so efficiency is not an issue. */

  for (gen=0; gen<NUM_GENERATORS; gen++) {
    index = table_generators[gen][0];
    strcpy((char *)triangle_table[index], (char *)table_generators[gen]+1);
    Do_all_orientations(table_generators[gen][0]);
  }

  /* Generate the edge table from the triangle table by deleting all the
   * repeated edges. */

  Generate_edge_table();
  
  /*  Display the triangle table  */
  if(0) {
    printf ("Triangle table and corresponding edge table:\n");
    printf ("--------------------------------------------\n");
    for (index=0; index<=255; index++) { 
      printf ("\ntriangle_table[%3d] = ",index); 
      Print_list(triangle_table[index]);
      printf ("\n    edge_table[%3d] = ",index);
      Print_list(edge_table[index]);
    }
    printf ("\n"); 
  }
  /* I have checked the generated table against Lorensen's table which he
   * includes in the Visualization Toolkit Version 1.0. Randomly comparing
   * both tables and specifically looking at 'awkward' cases confirms that
   * they are essentially the same. He uses a different labeling scheme and
   * makes different topologically arbitrary choices to his own paper. He
   * uses a similar modification to the case table to deal with holes. I
   * follow the labeling scheme in his paper and the modifications suggested
   * by Montani et al.  */
}
/* ========================================================================= */

static CubeIndex Rotate_edge_list (CubeIndex index, int direction)
{
  /* This takes an entry in the triangle_table and generates the new list of
   * triangles obtained by rotating the cube around either the X or the Y
   * axis. */

  CubeEdge    *mod_edge;
  CubeVertex  *mod_vertex;
  CubeIndex    new_index = 0;
  CubeEdge     new_edge_list[MAX_TRIANGLE_LIST_LENGTH];
  int          i;

  if (direction == 'x') {
    mod_edge   = x_mod_edge;
    mod_vertex = x_mod_vertex;
  }
  else {
    mod_edge   = y_mod_edge;
    mod_vertex = y_mod_vertex;
  }

  for (i=0; i<=7; i++) {
    if (index & (1<<i)) new_index |= mod_vertex[i+1];
  }

  i = 0;
  while (triangle_table[index][i] != END) {
    new_edge_list[i] = mod_edge[triangle_table[index][i]];
    i++;
  }
  new_edge_list[i] = END;

  if (triangle_table[new_index][0] == END) {
    /*  The first time this entry has been defined  */
    strcpy((char *)triangle_table[new_index], (char *)new_edge_list);
  }

  else {
    if (strcspn ((char *)triangle_table[new_index], (char *)new_edge_list) 
	!= 0) {
      /* Triangle lists are considered equivalent if they have the same
       * elements (even if they are in a different order).  I know this is a
       * weak test but its better than nothing. */
      fprintf (stderr,"Logic error generating lookup table!\n");
    }
  }

  return (new_index);
}
/* ========================================================================= */

static CubeIndex Rotate_edge_list_x (CubeIndex index)
{
  return (Rotate_edge_list(index,'x'));
}
/* ========================================================================= */

static CubeIndex Rotate_edge_list_y (CubeIndex index)
{
  return (Rotate_edge_list(index,'y'));
}
/* ========================================================================= */

static void Rotate_diagonal (CubeIndex index)
{
  CubeIndex  lindex = index;
  int        i;

  for (i=0; i<=2; i++) lindex = Rotate_edge_list_x(Rotate_edge_list_y(lindex));
  if (lindex != index)
    fprintf(stderr,"Assertion failed in Rotate_diagonal\n");
}
/* ========================================================================= */

static void Do_all_orientations (CubeIndex index)
{
  /* Given an entry in the triangle_table at location index, this function
   * rotates the cube through all 24 orientations and writes the resulting
   * edge lists to the edge table. */

  CubeIndex  lindex = index;   /* local copy of index */
  int        i;

  for (i=0; i<=3; i++) {
    Rotate_diagonal(lindex);
    lindex = Rotate_edge_list_x(lindex);
  }
  if (lindex != index)
    fprintf(stderr,"Assertion 1 failed in Do_all_orientations\n");
  
  lindex = Rotate_edge_list_y(Rotate_edge_list_y(lindex));
  for (i=0; i<=3; i++) {
    Rotate_diagonal(lindex);
    lindex = Rotate_edge_list_x(lindex);
  }
  lindex = Rotate_edge_list_y(Rotate_edge_list_y(lindex));
  if (lindex != index)
    fprintf(stderr,"Assertion 2 failed in Do_all_orientations\n");
}
/* ========================================================================= */

static void Generate_edge_table (void)
{
  /* This takes the triangle table and generates the edge_table by deleting
   * all the repeated vertices. */

  CubeIndex  index;
  int        pos1, pos2;
  Bool       got_already;
  CubeEdge   new_edge;

  for (index = 0; index<=255; index++) {
    
    /* go through this entry and write any new edges to the edge table */
    pos1 = 0;
    edge_table[index][0] = END;

    while ((new_edge = triangle_table[index][pos1]) != END) {

      pos2 = 0;
      got_already = FALSE;

      while (edge_table[index][pos2] != END) {
	if (new_edge == edge_table[index][pos2]) got_already = TRUE;
	pos2++;
      }

      if (!got_already) {
	edge_table[index][pos2] = new_edge;
	edge_table[index][pos2+1] = END;
      }

      pos1++;
    }
  }
}
/* ========================================================================= */

static void Print_list (CubeEdge *p)
{
  /* Simple routine which prints a list of edges as stored in the triangle
   * table and the edge table. */

  if (*p == END) return;                       /* Empty list */
  while (*(p+1) != END) printf ("%d, ",*p++);  /* Print all but the last */
  printf ("%d",*p);                            /* Print the last */
}
/* ========================================================================= */



