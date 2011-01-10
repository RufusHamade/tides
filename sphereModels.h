/* Data structures used to define models. */

#include "math3d.h"

typedef struct sm_chunk {
    GLenum type;
    int nVertices;
    int *vertices;
    bool backwards;
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
	// Increment an index within a given level.  When we reach the end of the level, wrap to the beginning.
	int incIdx(int idx, int step) {
		idx += step;
		if (idx >= firstVertex + nVertices)
			idx -= nVertices;
		if (idx < firstVertex)
			idx += nVertices;
		return idx;
	}
} sm_level;

typedef struct sm_tri {
	int indicesStart;
} sm_tri;

typedef struct sm_vertexInfo {
	int nTris;
	int tris[6];
	
	void addTri(int triIdx) {
#ifdef DEBUG
		if (nTris == 6) {
			printf("TOO MANY TRIS\n");
			system.exit();
		}
#endif

		tris[nTris] = triIdx;
		nTris++;
	}
} sm_vertexInfo;

typedef struct sm_model {
    int nVertices;
    M3DVector3f   *vertices;
	sm_vertexInfo *vInfo;
	
	int nLayers;
	sm_layer *layers;
	sm_level *levels;

    int nChunks;
    sm_chunk *chunks;

	int nTris;
	int nextTri;
	sm_tri   *tris;
	unsigned *indices;

	sm_chunk *getChunk(int layer, int offset) {
		sm_layer *l = &layers[layer]; 
		return &chunks[l->firstChunk + offset];
	}
	
	void addTri(int v0idx, int v1idx, int v2idx, bool backwards) {
		tris[nextTri].indicesStart = nextTri * 3;
		indices[nextTri * 3]     = v0idx;
		if (backwards) {
			indices[nextTri * 3 + 1] = v2idx;
			indices[nextTri * 3 + 2] = v1idx;
		}
		else {
			indices[nextTri * 3 + 1] = v1idx;
			indices[nextTri * 3 + 2] = v2idx;
		}
		vInfo[v0idx].addTri(nextTri);
		vInfo[v1idx].addTri(nextTri);
		vInfo[v2idx].addTri(nextTri);
		nextTri++;
	}
} sm_model;

void sm_freeModel(sm_model*m);
void sm_renderChunk(sm_model *m, sm_chunk *c);
sm_model *sm_getUnitIsocahedron();
sm_model *sm_getUnitSphere(int precision);
