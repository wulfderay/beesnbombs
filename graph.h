#ifndef __GRAPH_H
#define __GRAPH_H

#include <sys/types.h>
/***
Graph.h: a bit of a higher-level library to go on top of the basic one for psp.

Not a replacement for the "advanced" gs lib, just a couple of convenience functions for makeing boxes and the like.

***/
// Can c structs have functions?
typedef struct {
    // so we will need coordinates to draw from, a set of faces each of which is 2 triangles.
    // it would be really great to be able to figure out back-face culling using the triangle vertex order, but later.
    
    SVECTOR position;
    // a matrix?
    // maybe some materials, mabye not.
    
} BOX_T;


char * GRAPH_draw_box(BOX_T* box, int * ot, char * nextPri)
{
    
    // do magic that makes the box go in the ot.
    return nextPri;
}
#endif
