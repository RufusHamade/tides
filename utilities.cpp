#include <glew.h>

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "utilities.h"
#include "math3d.h"

void tu_checkError(const char *msg) {
	GLenum e = glGetError();
	int i = 0;
	while (e != GL_NO_ERROR) {
		printf("%s: error(%d): %s\n", msg, i, gluErrorString(e));
		i++;
		e = glGetError();
	}
}

M3DVector3f cubeV[]    = {{1.0f,  1.0f,  1.0f}, // POS X
                         { 1.0f,  1.0f, -1.0f},
                         { 1.0f, -1.0f, -1.0f},
                         { 1.0f, -1.0f,  1.0f},

                         {-1.0f, -1.0f, -1.0f}, // NEG X
                         {-1.0f,  1.0f, -1.0f},
                         {-1.0f,  1.0f,  1.0f},
                         {-1.0f, -1.0f,  1.0f},

                         { 1.0f,  1.0f,  1.0f}, // POS Y
                         {-1.0f,  1.0f,  1.0f},
                         {-1.0f,  1.0f, -1.0f},
                         { 1.0f,  1.0f, -1.0f},

                         {-1.0f, -1.0f, -1.0f}, // NEG Y
                         {-1.0f, -1.0f,  1.0f},
                         { 1.0f, -1.0f,  1.0f},
                         { 1.0f, -1.0f, -1.0f},

                         { 1.0f,  1.0f,  1.0f}, // POS Z
                         { 1.0f, -1.0f,  1.0f},
                         {-1.0f, -1.0f,  1.0f},
                         {-1.0f,  1.0f,  1.0f},

                         {-1.0f, -1.0f, -1.0f}, // NEG Z
                         { 1.0f, -1.0f, -1.0f},
                         { 1.0f,  1.0f, -1.0f},
                         {-1.0f,  1.0f, -1.0f} };

M3DVector3f cubeN[]   = {{ 1.0f,  0.0f,  0.0f}, // POS X
                         {-1.0f,  0.0f,  0.0f}, // NEG X
                         { 0.0f,  1.0f,  0.0f}, // POS Y
                         { 0.0f, -1.0f,  0.0f}, // NEG Y
                         { 0.0f,  0.0f,  1.0f}, // POS Z
                         { 0.0f,  0.0f, -1.0f}};// NEG Z

unsigned cubetris[]   = { 0,  3,  1, // POS X
                          2,  1,  3,
                          4,  7,  5, // NEG X
                          6,  5,  7,
                          8, 11,  9, // POS Y
                         10,  9, 11,
                         12, 15, 13, // NEG Y
                         14, 13, 15,
                         16, 19, 17, // POS Z
                         18, 17, 19,
                         20, 23, 21, // NEG Z
                         22, 21, 23};

tu_model_t *tu_getCube() {
  // Create vertices for a cube.  We need to create seperate vertices for each face as the normals are different
  tu_model_t *model = (tu_model_t*)calloc(1, sizeof(tu_model_t));
  model->nVertices  = sizeof(cubeV)/sizeof(M3DVector3f);
  model->nIndices   = sizeof(cubetris)/sizeof(unsigned);
  model->vertices   = (M3DVector3f *)calloc(model->nVertices, sizeof(M3DVector3f));
  model->normals    = (M3DVector3f *)calloc(model->nVertices, sizeof(M3DVector3f));
  model->texcoords  = (M3DVector3f *)calloc(model->nVertices, sizeof(M3DVector3f));
  for (int i = 0; i < model->nVertices; i++) {
    m3dCopyVector3(model->vertices[i],  cubeV[i]);
    m3dCopyVector3(model->normals[i],   cubeN[i/4]);
    m3dCopyVector3(model->texcoords[i], cubeV[i]);
  }
  model->indices    = (unsigned *)calloc(model->nIndices, sizeof(unsigned));
  memcpy(model->indices, cubetris, sizeof(cubetris));
  return model;
}

void tu_freeModel(tu_model_t *model) {
  if (model->vertices != NULL)
    free(model->vertices);
  if (model->normals != NULL)
    free(model->normals);
  if (model->texcoords != NULL)
    free(model->texcoords);
  if (model->indices != NULL)
    free(model->indices);
  free(model);
}

void tu_setColorForHeat(M3DVector3f color, float heat) {
	color[0] = (heat + 1.0f) / 2.0f;
	color[1] = 0.5f - fabsf(heat)/2;
	color[2] = 1.0f - color[0];
}

void tu_renderFromVertexList(int nVertices, M3DVector3f *vertices, M3DVector3f *normals, M3DVector3f *texcoords, M3DVector3f *colors, int nIndices, unsigned *indices) {
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, vertices);
	glNormalPointer(GL_FLOAT, 0, normals);
  if (texcoords != NULL) {
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(3, GL_FLOAT, 0, texcoords);
  }
  if (colors != NULL) {
    glEnableClientState(GL_COLOR_ARRAY);
    glColorPointer(3, GL_FLOAT, 0, colors);
  }
  else {
    glColor4fv(white);
  }
	tu_checkError("After set pointers");
	glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, indices);
	tu_checkError("Draw elements");
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
  if (texcoords != NULL) {
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  }
  if (colors != NULL) {
    glDisableClientState(GL_COLOR_ARRAY);
  }
}

void tu_loadShaderFromFile(GLuint id, const char* name) {
  struct stat st;
  if (stat(name, &st)) {
    printf("Coudn't stat file %s: %s\n", name, strerror(errno));
    exit(-1);
  }

  char *buf = (char*)malloc(st.st_size + 1);
  int fd = open(name, O_RDONLY);
  if (fd < 0) {
    printf("Coudn't open file %s: %s\n", name, strerror(errno));
    exit(-1);
  }

  int red = 0;
  while (red < st.st_size) {
    int last = read(fd, buf + red, st.st_size - red);
    if (last < 0) {
      printf("Coudn't read file %s: %s\n", name, strerror(errno));
      exit(-1);
    }
    red += last;
  }
  buf[red] = '\0';
  close(fd);

  tu_loadShader(id, name, buf);

  free(buf);
}

void tu_loadShader(GLuint id, const char* name, const char* shader) {
  const GLchar* frags[1];
  frags[0] = shader;
  glShaderSource(id, 1, frags, NULL);
  glCompileShader(id);
  GLint result;
  glGetShaderiv(id, GL_COMPILE_STATUS, &result);
  if (!result) {
    GLint len;
    glGetShaderiv(id, GL_INFO_LOG_LENGTH, &len);
    GLchar info[len];
    glGetShaderInfoLog(id, len, NULL, info);
    printf("Error compiling shader %s:\n", name);
    printf("%s\n", info);
    exit(-1);
  }
 	tu_checkError("After compile shader");
}

void tu_linkProgram(GLuint pId) {
  glLinkProgram(pId);
  GLint result;
  glGetProgramiv(pId, GL_LINK_STATUS, &result);
  if (!result) {
    GLint len;
    glGetProgramiv(pId, GL_INFO_LOG_LENGTH, &len);
    GLchar info[len];
    glGetProgramInfoLog(pId, len, NULL, info);
    printf("Error linking program:\n");
    printf("%s\n", info);
    exit(-1);
  }
 	tu_checkError("After link");
}

void tu_validateProgram(GLuint pId) {
  glValidateProgram(pId);
  GLint result;
  glGetProgramiv(pId, GL_VALIDATE_STATUS, &result);
  if (!result) {
    GLint len;
    glGetProgramiv(pId, GL_INFO_LOG_LENGTH, &len);
    GLchar info[len];
    glGetProgramInfoLog(pId, len, NULL, info);
    printf("Error validating program:\n");
    printf("%s\n", info);
    exit(-1);
  }
 	tu_checkError("After validate program");
}
