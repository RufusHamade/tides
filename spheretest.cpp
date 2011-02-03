#include <gl.h>
#include <glu.h>
#include <glut.h>

#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "sphereModels.h"

const float SCALE = 10.0f;
const int NMODELS = 5;

float angle = 0.0f;

M3DVector4f lightPos  = { 100.0f, 100.0f, 50.0f, 1.0f };  // Point source
M3DVector4f black     = { 0.0f, 0.0f, 0.0f, 1.0f };
M3DVector4f white     = { 1.0f, 1.0f, 1.0f, 1.0f };
M3DVector4f dim       = { 0.25f, 0.25f, 0.25f, 1.0f };
M3DVector4f bright    = { 1.0f, 1.0f, 1.0f, 1.0f };
					
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
	glColor3fv(white );
    for (int i = 0; i < m->nChunks; i++) {
		sm_renderChunk(m, &m->chunks[i]);
	}
}

void setColorForHeat(M3DVector3f color, float heat) {
	color[0] = (heat + 1.0f) / 2.0f;
	color[1] = 0.5f - fabsf(heat)/2;
	color[2] = 1.0f - color[0];
}

void checkError(const char *msg) {
	GLenum e = glGetError();
	int i = 0;
	while (e != GL_NO_ERROR) {
		printf("%s: error(%d): %s\n", msg, i, gluErrorString(e));
		i++;
		e = glGetError();
	}
}


void renderFromVertexList(sm_model_t *m) {
	M3DVector3f colors[m->nVertices];
	for (int i = 0; i < m->nVertices; i++) {
		setColorForHeat(colors[i], -2.0f*i/(m->nVertices - 1) + 1.0f);
	}
	
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, m->vertices);
	glNormalPointer(GL_FLOAT, 0, m->vertices);
	glColorPointer(3, GL_FLOAT, 0, colors);
	checkError("After set pointers");
	glDrawElements(GL_TRIANGLES, m->nTris * 3, GL_UNSIGNED_INT, m->indices);
	checkError("Draw elements");
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	
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
    glDisable(GL_LIGHT0);
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
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, black);
    glLightfv(GL_LIGHT0, GL_AMBIENT, dim);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, bright);
    glLightfv(GL_LIGHT0, GL_SPECULAR, bright);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    glMateriali(GL_FRONT, GL_SHININESS, 128);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

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
		glPushMatrix();
			glColor3f(0.75f, 0.75f, 0.75f);
			glTranslatef(2.5f*(i - (NMODELS - 1.0f)/2.0f), -2.5f, 0.0f);
			glRotatef(angle, 0.0f, 1.0f, 0.0f);
			glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
			renderFromVertexList(models[i]);
		glPopMatrix();
	}

	for (int i = 0; i < NMODELS; i++)
		sm_freeModel(models[i]);

	// Flush drawing commands
	glutSwapBuffers();
	angle = angle + 1;
	glutPostRedisplay();
}

// This function does any needed initialization on the rendering
// context. 
void SetupRC() {
	GLfloat white[] = {1.0f, 1.0f, 1.0f, 1.0f};
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, white);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f );
}

void ChangeSize(int w, int h) {
	// Calculate new clipping volume
	GLfloat windowWidth;
	GLfloat windowHeight;

	// Prevent a divide by zero, when window is too short
	// (you cant make a window of zero width).
	if(h == 0)
		h = 1;

	// Keep the square square
	if (w <= h) {
		windowHeight = SCALE*(GLfloat)h/(GLfloat)w;
		windowWidth = SCALE;
	}
    else {
		windowWidth = SCALE*(GLfloat)w/(GLfloat)h;
		windowHeight = SCALE;
	}

    // Set the viewport to be the entire window
    glViewport(0, 0, w, h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

	// Set the clipping volume
	glOrtho(-windowWidth/2, windowWidth/2, -windowHeight/2, windowHeight/2, -SCALE/2, SCALE/2);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

int main(int argc, char* argv[]) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Sphere Tesselation Test");
	glutReshapeFunc(ChangeSize);
	glutDisplayFunc(RenderScene);

	SetupRC();

	glutMainLoop();
	return 0;
}
