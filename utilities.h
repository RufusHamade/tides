#ifndef _TIDES_UTILITIES_H
#define _TIDES_UTILITIES_H

#include <gl.h>

#include "math3d.h"

const M3DVector4f black     = { 0.0f, 0.0f, 0.0f, 1.0f };
const M3DVector4f white     = { 1.0f, 1.0f, 1.0f, 1.0f };
const M3DVector4f dim       = { 0.25f, 0.25f, 0.25f, 1.0f };
const M3DVector4f bright    = { 1.0f, 1.0f, 1.0f, 1.0f };
const M3DVector4f green     = {0.0f, 1.0f, 0.0f};

typedef struct {
  int            nVertices;
  M3DVector3f   *vertices;
  M3DVector3f   *normals;
  M3DVector3f   *texcoords;
  int            nIndices;
	unsigned      *indices;
}
tu_model_t;

void tu_checkError(const char *msg);

tu_model_t *tu_getCube();
void tu_freeModel(tu_model_t *model);

void tu_renderFromVertexList(int nVertices, M3DVector3f *vertices, M3DVector3f *normals, M3DVector3f *texcoords, M3DVector3f *colors, int nIndices, unsigned *indices);

void tu_setColorForHeat(M3DVector3f color, float heat);

void tu_loadShaderFromFile(GLuint id, const char* name);
void tu_loadShader(GLuint id, const char* name, const char* shader);
void tu_linkProgram(GLuint pId);
void tu_validateProgram(GLuint pId);

#endif
