#include <sys/types.h>	// This provides typedefs needed by libgte.h and libgpu.h
#include <stdio.h>	// Not necessary but include it anyway
// #include <libetc.h>	// Includes some functions that controls the display
// #include <libgte.h>	// GTE header, not really used but libgpu.h depends on it
// #include <libgpu.h>	// GPU library header
/*
Implements the 5-order polynomial approximation to sin(x).
@param i   angle (with 2^15 units/circle)
@return    16 bit fixed point Sine value (4.12) (ie: +4096 = +1 & -4096 = -1)

The result is accurate to within +- 1 count. ie: +/-2.44e-4.
*/
u_short fpsin(u_short i);

//Cos(x) = sin(x + pi/2)
#define fpcos(i) fpsin((int)(((int)(i)) + 8192U))