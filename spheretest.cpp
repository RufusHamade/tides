#include <GL/gl.h>
#include <GL/glu.h>
#include <glut.h>

#include <math.h>
#include <stdlib.h>

#include "sphereModels.h"

#define SCALE 8.0f

float angle = 0.0f;

// Called to draw scene
void RenderScene(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// Draw three spheres at various levels of improvement side-by-side.
	for (int i = 0; i < 5; i++) {
		glPushMatrix();
		glTranslatef(2.5f*(i-2), 0.0f, 0.0f);
		glRotatef(angle, 0.0f, 1.0f, 0.0f);
		glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
		sm_model *s= sm_getUnitSphere(i);
		sm_renderModel(s);
		sm_freeModel(s);
		glPopMatrix();
	}

	// Flush drawing commands
	glutSwapBuffers();
	angle = angle + 1;
	glutPostRedisplay();
}

// This function does any needed initialization on the rendering
// context. 
void SetupRC() {
	// Black background
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
