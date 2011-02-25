#include <glew.h>
#include <glut.h>

#include <stdlib.h>
#include <stdio.h>

#include "math3d.h"
#include "utilities.h"

const float SCALE   = 4.0f;

M3DVector3f tri[]    = {{  0.0f,  1.0f, 0.0f},
                        { -1.0f, -1.0f, 0.0f},
                        {  1.0f, -1.0f, 0.0f}};
GLuint indices[] = {0, 1, 2};

GLuint VSID_simple;
GLuint VSID_attribute;

GLuint FSID_simple;

GLuint PID_simple;
GLuint PID_attribute;

const GLuint AID_vertex = 0;

GLuint VAO;

GLuint VBUFID_vertex;
GLuint IBUFID_index;

const char* vs_simple =
"#version 130\n"
"void main(void) {\n"
" gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
"}\n";

const char* fs_simple =
"#version 130\n"
"void main(void) {\n"
"  gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);\n"
"}\n";

const char* vs_attribute =
"#version 130\n"
"in vec4 vertex;\n"
"void main(void) {\n"
" gl_Position = gl_ModelViewProjectionMatrix * vertex;\n"
"}\n";

typedef enum {
  FIXED,
  SHADER_SIMPLE,
  SHADER_WITH_ATTRIBUTES
} pipelinetype_t;

typedef enum {
  IMMEDIATE,
  BULK,
  BUFFERED
} rendertype_t;

pipelinetype_t pipelineType   = FIXED;
rendertype_t   renderType     = IMMEDIATE;

void RenderScene_immediate_builtin(void) {
  printf("immediate via builtin\n");
  glBegin(GL_TRIANGLES);
  for (unsigned i = 0; i < sizeof(indices)/sizeof(GLuint); i++)
    glVertex3fv(tri[i]);
  glEnd();
}

void RenderScene_immediate_attributes(void) {
  printf("immediate via attribute\n");
  glBegin(GL_TRIANGLES);
  for (unsigned i = 0; i < sizeof(indices)/sizeof(GLuint); i++)
    glVertexAttrib3fv(AID_vertex, tri[i]);
  glEnd();
}

void RenderScene_bulk_builtin(void) {
  printf("bulk via builtin\n");
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, tri);
 	glDrawElements(GL_TRIANGLES, sizeof(indices)/sizeof(GLuint), GL_UNSIGNED_INT, indices);
	glDisableClientState(GL_VERTEX_ARRAY);
}

void RenderScene_bulk_attributes(void) {
  printf("bulk via attribute\n");
  glEnableVertexAttribArray(AID_vertex);
	glVertexAttribPointer(AID_vertex, 3, GL_FLOAT, GL_FALSE, 0, tri);
 	glDrawElements(GL_TRIANGLES, sizeof(indices)/sizeof(GLuint), GL_UNSIGNED_INT, indices);
  glDisableVertexAttribArray(AID_vertex);
}

void RenderScene_buffered_builtin(void) {
  printf("buffered via builtin\n");
	glEnableClientState(GL_VERTEX_ARRAY);

  glBindBuffer(GL_ARRAY_BUFFER, VBUFID_vertex);
	glVertexPointer(3, GL_FLOAT, 0, 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBUFID_index);
 	glDrawElements(GL_TRIANGLES, sizeof(indices)/sizeof(GLuint), GL_UNSIGNED_INT, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glDisableClientState(GL_VERTEX_ARRAY);
}

void RenderScene_buffered_attributes(void) {
  printf("buffered via attribute\n");
  glEnableVertexAttribArray(AID_vertex);

  glBindBuffer(GL_ARRAY_BUFFER, VBUFID_vertex);
  glVertexAttribPointer(AID_vertex, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBUFID_index);
 	glDrawElements(GL_TRIANGLES, sizeof(indices)/sizeof(GLuint), GL_UNSIGNED_INT, 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glDisableVertexAttribArray(AID_vertex);
}

void RenderScene(void) {

  switch(pipelineType) {
    case FIXED:
      printf("FIXED ");
      glUseProgram(0);
      break;
    case SHADER_SIMPLE:
      printf("SHADER: SIMPLE ");
      tu_validateProgram(PID_simple);
      glUseProgram(PID_simple);
      break;
    case SHADER_WITH_ATTRIBUTES:
      printf("SHADER: ATTRIB ");
      tu_validateProgram(PID_attribute);
      glUseProgram(PID_attribute);
      break;
  }

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  if (renderType == IMMEDIATE) {
    if (pipelineType == SHADER_WITH_ATTRIBUTES)
      RenderScene_immediate_attributes();
    else
      RenderScene_immediate_builtin();
  }
  else if (renderType == BULK) {
    if (pipelineType == SHADER_WITH_ATTRIBUTES)
      RenderScene_bulk_attributes();
    else
      RenderScene_bulk_builtin();
  }
  else {
    if (pipelineType == SHADER_WITH_ATTRIBUTES)
      RenderScene_buffered_attributes();
    else
      RenderScene_buffered_builtin();
  }

  glutSwapBuffers();
  tu_checkError("After RenderScene");
}

void SetupRC() {
  glEnable(GL_DEPTH_TEST);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glFrontFace(GL_CCW);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f );

  PID_simple    = glCreateProgram();
  PID_attribute = glCreateProgram();

  VSID_simple    = glCreateShader(GL_VERTEX_SHADER);
  VSID_attribute = glCreateShader(GL_VERTEX_SHADER);
  FSID_simple    = glCreateShader(GL_FRAGMENT_SHADER);
  tu_loadShader(VSID_simple,    "simple vertex",    vs_simple);
  tu_loadShader(VSID_attribute, "attribute vertex", vs_attribute);
  tu_loadShader(FSID_simple,    "simple fragment",  fs_simple);

  glAttachShader(PID_simple, VSID_simple);
  glAttachShader(PID_simple, FSID_simple);
  tu_linkProgram(PID_simple);

  glAttachShader(PID_attribute, VSID_attribute);
  glAttachShader(PID_attribute, FSID_simple);
  glBindAttribLocation(PID_attribute, AID_vertex, "vertex");
  tu_linkProgram(PID_attribute);

  //glGenVertexArrays(1, &VAO);
  //glBindVertexArray(VAO);

  glGenBuffers(1, &VBUFID_vertex);
  glGenBuffers(1, &IBUFID_index);

  glBindBuffer(GL_ARRAY_BUFFER, VBUFID_vertex);
  glBufferData(GL_ARRAY_BUFFER, sizeof(tri), tri, GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBUFID_index);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  tu_checkError("After SetupRC");
}

void ChangeSize(int w, int h) {
  GLfloat windowWidth, windowHeight;

  if(h == 0)
    h = 1;

  if (w <= h) {
    windowHeight = SCALE*(GLfloat)h/(GLfloat)w;
    windowWidth = SCALE;
  }
  else {
    windowWidth = SCALE*(GLfloat)w/(GLfloat)h;
    windowHeight = SCALE;
  }

  glViewport(0, 0, w, h);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(-windowWidth/2, windowWidth/2, -windowHeight/2, windowHeight/2, -SCALE/2, SCALE/2);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  tu_checkError("After ChangeSize");
}

typedef struct {
  const char* tag;
  int *ptr;
  int value;
} args_t;

args_t args[] = {{"immediate", (int*)&renderType,   IMMEDIATE},
                 {"bulk",      (int*)&renderType,   BULK},
                 {"buffered",  (int*)&renderType,   BUFFERED},
                 {"FIXED",     (int*)&pipelineType, FIXED},
                 {"SIMPLE",    (int*)&pipelineType, SHADER_SIMPLE},
                 {"ATTRIB",    (int*)&pipelineType, SHADER_WITH_ATTRIBUTES}};

int main(int argc, char* argv[]) {
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
  glutInitWindowSize(800, 600);
  glutCreateWindow("Shader Test");
  glutReshapeFunc(ChangeSize);
  glutDisplayFunc(RenderScene);

  GLenum err = glewInit();
  if (GLEW_OK != err) {
    fprintf(stderr, "GLEW initialisation error: %s\n", glewGetErrorString(err));
    exit(1);
  }

  for (int i = 1; i < argc; i++) {
    for (unsigned j = 0; j < sizeof(args)/sizeof(args_t); j++) {
      if (strcmp(argv[i], args[j].tag) == 0) {
        *args[j].ptr = args[j].value;
        goto NEXT;
      }
    }
    printf("Ignoring unknown argument %s\n", argv[i]);
    NEXT:;
  }

  SetupRC();

  glutMainLoop();
  return 0;
}
