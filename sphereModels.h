/* Data structures used to define models. */

#include <GL/gl.h>
#include "math3d.h"

typedef struct sm_chunk {
    GLenum type;
    int nNodes;
    int *nodes;
} sm_chunk;

typedef struct sm_layer {
    int firstChunk;
    int nChunks;

    int followingChunk() {
		return firstChunk + nChunks;
	}
} sm_layer;

typedef struct sm_level {
    int firstVertex;
    int nVertices;
    
    int followingVertex() {
		return firstVertex + nVertices;
	}
} sm_level;

typedef struct sm_model {
    int nVertices;
    M3DVector3f  *vertices;
	
	int nLayers;
	sm_layer *layers;
	sm_level *levels;

    int nChunks;
    sm_chunk *chunks;

	sm_chunk *getChunk(int layer, int offset) {
		sm_layer *l = &layers[layer]; 
		return &chunks[l->firstChunk + offset];
	}
} sm_model;

void sm_freeModel(sm_model*m);
void sm_renderModel(sm_model*m);
sm_model *sm_getUnitIsocahedron();
sm_model *sm_getUnitSphere(int precision);
