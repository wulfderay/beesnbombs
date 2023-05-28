#include <sys/types.h>	// This provides typedefs needed by libgte.h and libgpu.h
#include <stdio.h>	// Not necessary but include it anyway
#include <libetc.h>	// Includes some functions that controls the display
#include <libgte.h>	// GTE header, not really used but libgpu.h depends on it
#include <libgpu.h>	// GPU library header
#include "psxpad.h" // defines for buttons.
#include "cube.h"

#define X_WIDTH 320
#define Y_HEIGHT 240
// fixed point unit
#define FIXED_POINT_ONE 4096
#define FIXED_POINT_BITS 12
#define FIXED_POINT_MANTISSA 0xfff

/* libgte functions return 'otz', which indicates the OT linking position.
 * 'otz' is a quater value of the Z in the screen cordinate, and libgte
 * has 15bit (0-0x7fff) z range. Therefore maximum OT size should be 4096
 * (2^14) */
#define SCR_Z	(512)		/* screen depth*/
#define	OTSIZE	(4096)		/* OT size*/
#define NCUBE	100		/* max number of cubes*/
int		ncube = 100;
int 	speed = 256;
int		wavelengthconst = 328; // 327.68 * 50 is the period

// Define environment pairs and buffer counter
DISPENV disp[2];
DRAWENV draw[2];
/* color of cube*/
CVECTOR		col[6];	

int ot[2][OTSIZE];

char pribuff[2][32768];
char *nextpri;

int tim_mode;
RECT tim_prect, tim_crect;
int tim_uoffs, tim_voffs;
int angle = 0;

/* angle of rotation*/
// values taken from in-game arangement.
static SVECTOR	ang  = { 320, -2592, 2112};	

/* translation vertex*/
static VECTOR	vec  = {0, 0, SCR_Z};	

/* scale of the each cube*/
static VECTOR	scale  = {1024, 1024, 1024, 0};

typedef struct {
	CVECTOR	col[6];			/* color of cube surface*/
	SVECTOR	trans;			/* translation vector (local coord.) */
} CUBEATTR;

typedef struct {
	POLY_F4		s[6];		/* surface of cube*/
} CUBE;



u_char padbuff[2][34]; 
typedef struct {
	DRAWENV		draw;		/* drawing environment*/
	DISPENV		disp;		/* display environment*/
	u_long		ot[OTSIZE];	/* OT*/
	CUBE		cubes[NCUBE];	/* pointer to CUBE*/
} DB;
/* double buffer*/
DB		db[2];	

#define MIN_D 	64		/* minumus distance between each cube */
#define MAX_D	(SCR_Z/2)	/* maximum distance */
/* CUBEATTR	*attr,	/* attribute of cube */
/* int		nattr	/* number of cube */
static void init_attr(CUBEATTR *attr, int nattr)
{
	int	i,j;
	int x,y,z;
	int nattr_orig = nattr;

	POLY_F4	templ;
	SetPolyF4(&templ);
	j = 0;
	for (; nattr; nattr--, attr++) {
		for (i = 0; i < 6; i++) {
			attr->col[i].cd = templ.code;	/* sys code */
			attr->col[i].r  = rand();	/* R */
			attr->col[i].g  = rand();	/* G */
			attr->col[i].b  = rand();	/* B */
		}
		/* Set initial coordinates */
		attr->trans.vx = (((nattr_orig - nattr) * 100) % 1000) - (500);
		attr->trans.vy = 0;
		attr->trans.vz = (((nattr_orig - nattr)/10) * 100 ) - 500;
	}
}

/*
 *	Initialization of Primitives */
/*DB	*db;	/* primitive buffer*/
/*CVECTOR *c;	/* coloer of cube surface*/
static void init_prim(DB *db, CVECTOR *c)
{
	int	i, j;

	/* initialize for side polygon*/
	for (i = 0; i < NCUBE; i++) 
	for (j = 0; j < 6; j++) {
		/* initialize POLY_FT4*/
		SetPolyF4(&db->cubes[i].s[j]);	
		setRGB0(&db->cubes[i].s[j], c[j].r, c[j].g, c[j].b);
	}
}

/* 
 *  Analyzing PAD and setting Matrix */
/* MATRIX *rottrans */
static int pad_read(MATRIX *rottrans)
{
	PADTYPE * pad;
	int padd;
	int	ret = 0;

	/* read from controller*/
    pad = (PADTYPE*)padbuff[0];
    	   if (pad -> stat == 0)
    	   {	
    		// only parse when using digital pad, dual-analog and dual-shock.
    		if ( pad -> type == PAD_TYPE_DIGITAL ||
    			pad -> type == PAD_TYPE_ANALOG ||
    			pad -> type == PAD_TYPE_DUAL)
    		{
    			if ( !(pad->btn & PADRup)) ang.vz += 32;
				if ( !(pad->btn & PADRdown))	ang.vz -= 32;
				if ( !(pad->btn & PADRleft)) 	ang.vy += 32;
				if ( !(pad->btn & PADRright))	ang.vy -= 32;
				if ( !(pad->btn & PAD_L1))	scale.vy+= 32;
				if ( !(pad->btn & PAD_L2))	scale.vy-= 32;
				if ( !(pad->btn & PAD_R1))	ang.vx+= 32;
				if ( !(pad->btn & PAD_R2))	ang.vx-= 32;
				if ( !(pad->btn & PAD_CROSS))	speed+= 32;
				if ( !(pad->btn & PAD_TRIANGLE))	speed-= 32;
				if ( !(pad->btn & PAD_SQUARE))	wavelengthconst+= 1;
				if ( !(pad->btn & PAD_CIRCLE))	wavelengthconst-= 1;
				
    		}
		}
	if (speed < 0 ) speed = 0;
	if (speed > 4096) speed = 4096; 

	/* print status*/
	FntPrint("tuto3: simple cube rot=(%d,%d,%d)\n speed=%d\n",
		 ang.vx, ang.vy, ang.vz, speed);
		
	return(ret);
}



void init(void)
{
	int i;
    // Reset GPU and enable interrupts
    ResetGraph(0);
    

	/* set surface colors */
	// for (i = 0; i < 6; i++) {
	// 	col[i].r = rand();	/* R */
	// 	col[i].g = rand();	/* G */
	// 	col[i].b = rand();	/* B */
	// }
    
    // Enable background clear
    db[0].draw.isbg = 1;
    db[1].draw.isbg = 1;
    

   
    
    // init the next primitive pointer to the start of buffer 0.
    nextpri = pribuff[0];
    
    // Load font texture on upper right of VRAM
    FntLoad(960,0);
    // Define a font window of 100 chars covering the whole screen
    FntOpen(0, 8, 320, 224, 0, 100);
    
	/*
	 * When using interrace mode, there is no need to changing 
	 * draw/display environment for every frames because the same
	 * same area is used for both drawing and displaying images.
	 * Therefore, the environment should be set only at the first time.
	 * Even in this case, 2 primitive buffers are needed since drawing
	 * process runs parallely with CPU adn GPU. */
	PutDrawEnv(&db[0].draw);	/* set up rendering environment*/
	PutDispEnv(&db[0].disp);	/* set up display environment*/
    
    InitPAD( padbuff[0], 34, padbuff[1], 34);
    padbuff[0][0] = padbuff[0][1] = 0xff; // init to ffff so no spurious input in the beginning.
    padbuff[1][0] = padbuff[1][1] = 0xff;
    
    
}

void init_system(int x, int y, int z, int level)
{
	init();
	/* reset graphic subsystem*/
	FntLoad(960, 256);	
	SetDumpFnt(FntOpen(32, 32, 320, 64, 0, 512));
	
	/* set debug mode (0:off, 1:monitor, 2:dump)  */
	SetGraphDebug(level);	

	/* initialize geometry subsystem*/
	InitGeom();			
	
	/* set geometry origin as (160, 120)*/
	SetGeomOffset(x, y);	
	
	/* distance to veiwing-screen*/
	SetGeomScreen(z);	
		
}

int main()
{
	/* double buffer*/
	DB		db[2];		
	
	/* current db*/
	DB		*cdb;		
	/* attribute of cube*/
	CUBEATTR	attr[NCUBE];
	
	/* rotation-translation matrix*/
	MATRIX		rottrans;	
	
	/* color of cube*/
	CVECTOR		col[6];		
	
	int		i,x,z,dist;		/* work */
	int		dmy, flg;	/* work */
	int phase = 0;
	/* scale of the each cube*/
	VECTOR	scale  = {1024, 1024, 1024, 0};
	

	/* initialize environment for double buffer (interlace) */
	/*	buffer #0	(0,  0)-(640,480)
	 *	buffer #1	(0,  0)-(640,480)
	 */
	init_system(320, 240, SCR_Z, 0);
	SetDefDrawEnv(&db[0].draw, 0, 0, 640, 480);
	SetDefDrawEnv(&db[1].draw, 0, 0, 640, 480);
	SetDefDispEnv(&db[0].disp, 0, 0, 640, 480);
	SetDefDispEnv(&db[1].disp, 0, 0, 640, 480);

	/* set surface colors */
	for (i = 0; i < 6; i++) {
		col[i].r = rand();	/* R */
		col[i].g = rand();	/* G */
		col[i].b = rand();	/* B */
	}

	/* set primitive parameters on buffer #0/#1 */
	init_prim(&db[0], col);	
	init_prim(&db[1], col);	
	init_attr(attr, NCUBE);
	/* enable to display*/
	SetDispMask(1);			

	/*
	 * When using interrace mode, there is no need to changing 
	 * draw/display environment for every frames because the same
	 * same area is used for both drawing and displaying images.
	 * Therefore, the environment should be set only at the first time.
	 * Even in this case, 2 primitive buffers are needed since drawing
	 * process runs parallely with CPU adn GPU. */
	PutDrawEnv(&db[0].draw);	/* set up rendering environment*/
	PutDispEnv(&db[0].disp);	/* set up display environment*/
	StartPAD();
	/* loop while [select] key*/
	while (pad_read(&rottrans) == 0) {	
		angle += speed;
		angle %= 32768;
		/* swap double buffer*/
		cdb = (cdb==db)? db+1: db;	
		
		/* clear OT.
		 * ClearOTagR() clears OT as reversed order. This is natural
		 * for 3D type application, because OT pointer to be linked
		 * is simply related to Z value of the primivie. ClearOTagR()
		 * is faster than ClearOTag because it uses hardware DMA
		 * channel to clear. */
		ClearOTagR(cdb->ot, OTSIZE);	
		// fpsin goes from -4096 to +4096.
		// it takes angles with 32768 being a full rotation.

		// so we want a distance from the center mapped to 
		// an angle...
		// and we want an x/z coordinate.
		/* add cubes to the OT
		 * note that only translation vector of the local
		 * screen matrix is changed  */ 
		x = 0;
		z = 0;
		dist = 0;
		for (i = 0; i < ncube; i++) {
			
			x++;
			if (x == 10)
			{
				x = 0;
				z++;
			}
			// x and z go from 0 to 10 each
			// which means that the answer
			// goes from .
			// we need to map that to 1/2 rotation
			// which should be ~16000/50. (327.68 exatly)
			dist = ((x - 5) * (x - 5)) + ((z - 5) * (z- 5) );
			//phase =(dist * 328)
			scale.vy =2048+ fpsin(angle + (dist * wavelengthconst)); //1024 + fpsin(phase);
			/* calculate matrix*/
			RotMatrix(&ang, &rottrans);	/* rotation*/
			TransMatrix(&rottrans, &vec);	/* translation*/
			ScaleMatrix(&rottrans, &scale);
			/* set matrix*/
			SetRotMatrix(&rottrans);		/* rotation*/
			SetTransMatrix(&rottrans);	/* translation*/
			RotTrans(&attr[i].trans, (VECTOR *)rottrans.t, &flg);
			add_cubeF4(cdb->ot, cdb->cubes[i].s, &rottrans);

		}


		/* When using interlaced single buffer, all drawing have to be
		 * finished in 1/60 sec. Therefore we have to reset the drawing
		 * procedure at the timing of VSync by calling ResetGraph(1)
		 * instead of DrawSync(0) */
		VSync(0);	
		ResetGraph(1);	

		/* clear background*/
		ClearImage(&cdb->draw.clip, 60, 120, 120);
		
		/* Draw Otag.
		 * Since ClearOTagR() clears the OT as reversed order, the top
		 * pointer of the table is ot[OTSIZE-1]. Notice that drawing
		 * start point is not ot[0] but ot[OTSIZE-1]. */
		/*DumpOTag(cdb->ot+OTSIZE-1);	/* for debug */
		DrawOTag(cdb->ot+OTSIZE-1);	
		FntFlush(-1);
	}
        /* close controller*/
	DrawSync(0);
	return;
}