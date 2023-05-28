/* $PSLibId: Run-time Library Release 4.4$ */
/*				cube
 *
 *         Copyright (C) 1993-1995 by Sony Computer Entertainment
 *			All rights Reserved
 */
/*		  	  Draw cubes */
#ifndef __CUBE_H
#define __CUBE_H
#include <sys/types.h>
#include <libetc.h>
#include <libgte.h>
#include <libgpu.h>

/*
 *	cube vertex database */
#define CUBESIZ	196		/* cube size*/

/* vertex */

/*
 * Calcurate the vertex of the cube and put the primitive which
 * represents a surface of cube */
/* u_long	*ot;		/* OT pointer				  */
/* int		otsize;		/* OT size 				  */
/* POLY_F4	*s;		/* primitive for cube */
/* MATRIX	*rottrans;	/* rot-trans matrix */
/* CVECTOR	*c;		/* surface color */

/*
 *	POLY_F4 cube */
void add_cubeF4(u_long *ot, POLY_F4 *s, MATRIX *rottrans);


/*
 *	POLY_F4 cube with lighting */
void add_cubeF4L(u_long *ot, POLY_F4 *s, MATRIX *rottrans, CVECTOR *c);


/*
 *	POLY_F4 cube with fog */
void add_cubeF4F(u_long *ot, POLY_F4 *s, MATRIX *rottrans, CVECTOR *c);

void add_cubeFT4L(u_long *ot, POLY_FT4 *s, MATRIX *rottrans, CVECTOR *c);

#if 0
/*
 *	POLY_FT4 cube */
void add_cubeFT4(u_long *ot, POLY_FT4 *s, MATRIX *rottrans);

#endif

#endif