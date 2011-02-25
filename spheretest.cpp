#include <glew.h>
#include <glut.h>

#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "sphereModels.h"
#include "utilities.h"

const float SCALE = 10.0f;
const int NMODELS = 5;

float angle = 0.0f;

const M3DVector4f lightPos  = { 100.0f, 100.0f, 50.0f, 1.0f };  // Point source

void renderColorfulModel(sm_model_t *m) {
  const int NCOLS = 7;
  M3DVector3f colors[NCOLS] = {{1.0f, 1.0f, 1.0f},
                               {1.0f, 0.0f, 0.0f},
                               {0.0f, 1.0f, 0.0f},
                               {0.0f, 0.0f, 1.0f},
                               {1.0f, 1.0f, 0.0f},
                               {1.0f, 0.0f, 1.0f},
                               {0.0f, 1.0f, 1.0f} };
  int color = 0;

  for (int i = 0; i < m->nChunks; i++) {
    glColor3fv(colors[color]);
    sm_renderChunk(m, &m->chunks[i]);
    color = (color + 1)%NCOLS;
  }
}

void renderModel(sm_model_t *m) {
  glColor3fv(white);
  for (int i = 0; i < m->nChunks; i++) {
    sm_renderChunk(m, &m->chunks[i]);
  }
}

// Called to draw scene
void RenderScene(void) {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  sm_model_t *models[NMODELS];
  for (int i = 0; i < NMODELS; i++)
    models[i] = sm_getUnitSphere(i);

  // Draw three spheres at various levels of improvement side-by-side.
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_LIGHTING);

  for (int i = 0; i < NMODELS; i++) {
    glPushMatrix();
      glTranslatef(2.5f*(i - (NMODELS - 1.0f)/2.0f), 2.5f, 0.0f);
      glRotatef(angle, 0.0f, 1.0f, 0.0f);
      glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
      renderModel(models[i]);
    glPopMatrix();
  }

  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LIGHTING);

  for (int i = 0; i < NMODELS; i++) {
    glPushMatrix();
      glColor3f(0.75f, 0.75f, 0.75f);
      glTranslatef(2.5f*(i - (NMODELS - 1.0f)/2.0f), 0.0f, 0.0f);
      glRotatef(angle, 0.0f, 1.0f, 0.0f);
      glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
      renderColorfulModel(models[i]);
    glPopMatrix();
  }

  for (int i = 0; i < NMODELS; i++) {
    sm_model_t *m = models[i];
    M3DVector3f colors[m->nVertices];
    for (int j = 0; j < m->nVertices; j++) {
      tu_setColorForHeat(colors[j], -2.0f*j/(m->nVertices - 1) + 1.0f);
    }

    glPushMatrix();
      glColor3f(0.75f, 0.75f, 0.75f);
      glTranslatef(2.5f*(i - (NMODELS - 1.0f)/2.0f), -2.5f, 0.0f);
      glRotatef(angle, 0.0f, 1.0f, 0.0f);
      glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
      tu_renderFromVertexList(m->nVertices, m->vertices, m->vertices, NULL, colors, m->nTris*3, m->indices);
    glPopMatrix();
  }

  for (int i = 0; i < NMODELS; i++)
    sm_freeModel(models[i]);

  glutSwapBuffers();
  angle = angle + 1;
  glutPostRedisplay();
  tu_checkError("After RenderScene");
}

// This function does any needed initialization on the rendering
// context.
void SetupRC() {
  glCullFace(GL_BACK);
  glFrontFace(GL_CCW);

	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, black);
  glEnable(GL_LIGHT0);
  glLightfv(GL_LIGHT0, GL_AMBIENT, dim);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, bright);
  glLightfv(GL_LIGHT0, GL_SPECULAR, bright);
  glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

  glEnable(GL_COLOR_MATERIAL);
  glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
  glMateriali(GL_FRONT, GL_SHININESS, 128);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f );
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

int main(int argc, char* argv[]) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Sphere Tesselation Test");
	glutReshapeFunc(ChangeSize);
	glutDisplayFunc(RenderScene);

  GLenum err = glewInit();
  if (GLEW_OK != err) {
    fprintf(stderr, "GLEW initialisation error: %s\n", glewGetErrorString(err));
    exit(1);
  }

	SetupRC();

	glutMainLoop();
	return 0;
}
