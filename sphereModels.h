/* Data structures used to define models. */

#include <gl.h>
#include "math3d.h"

#ifndef _TIDES_SPHERE_MODELS_H
#define _TIDES_SPHERE_MODELS_H

typedef struct {
  GLenum type;
  int nVertices;
  int *vertices;
  bool backwards;
} sm_chunk_t;

typedef struct {
  int firstChunk;
  int nChunks;

  int followingChunk() {
    return firstChunk + nChunks;
  }
} sm_layer_t;

typedef struct {
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
} sm_level_t;

typedef struct {
  int indicesStart;
} sm_tri_t;

typedef struct {
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
} sm_vertexInfo_t;

typedef struct {
  int nVertices;
  M3DVector3f     *vertices;
  sm_vertexInfo_t *vInfo;

  int nLayers;
  sm_layer_t *layers;
  sm_level_t *levels;

  int nChunks;
  sm_chunk_t *chunks;

  int nTris;
  int nextTri;
  sm_tri_t   *tris;
  unsigned *indices;

  sm_chunk_t *getChunk(int layer, int offset) {
    sm_layer_t *l = &layers[layer];
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
} sm_model_t;

void sm_freeModel(sm_model_t*m);
void sm_renderChunk(sm_model_t *m, sm_chunk_t *c);
sm_model_t *sm_getUnitIsocahedron();
sm_model_t *sm_getUnitSphere(int precision);

#endif /* _TIDES_SPHERE_MODELS_H */
