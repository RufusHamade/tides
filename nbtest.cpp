#include <gl.h>
#include <glu.h>
#include <glut.h>

#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "sphereModels.h"
#include "nbody.h"

const float SCALE = 30.0f;
const float DT    = 0.1f;
const float HEAT_SCALE = 30.0f;

M3DVector4f lightPos  = { 100.0f, 100.0f, 50.0f, 1.0f };  // Point source
M3DVector4f black     = { 0.0f, 0.0f, 0.0f, 1.0f };
M3DVector4f white     = { 1.0f, 1.0f, 1.0f, 1.0f };
M3DVector4f dim       = { 0.25f, 0.25f, 0.25f, 1.0f };
M3DVector4f bright    = { 1.0f, 1.0f, 1.0f, 1.0f };

nb_world_t *world = NULL;
float initialEnergy;
int i = 0;

void setColorForHeat(M3DVector3f color, float heat) {
	heat *= HEAT_SCALE;
	if (heat < -1.0f) heat = -1.0f;
	if (heat > 1.0f)  heat = 1.0f;
	color[0] = (heat + 1.0f) / 2.0f;
	color[1] = 0.5f - fabsf(heat)/2;
	color[2] = 1.0f - color[0];
}

void renderModel(nb_world_t *world, int body) {
	nb_body_t  *b = &world->bodies[body];
	sm_model_t *s = b->unitSphere;
	M3DVector3f colors[s->nVertices];
	
	for (int i = 0; i < s->nVertices; i++) {
		setColorForHeat(colors[i], b->pfNormalComponent[i]);
	}
	
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, b->displayVertices);
	glNormalPointer(GL_FLOAT, 0, b->displayNormals);
	glColorPointer(3, GL_FLOAT, 0, colors);
	glDrawElements(GL_TRIANGLES, s->nTris * 3, GL_UNSIGNED_INT, s->indices);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
}

// Called to draw scene
void RenderScene(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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

	
	if (i%1000 == 0) {
		float mtot, etot;
		M3DVector3f vtot, com;
		nb_getSummaryValues(mtot, com, vtot, etot, world);
		printf("Totals @ %-10.4g: %10.3g %10.3g %10.3g %10.3g\n", world->t, etot, etot - initialEnergy, m3dGetVectorLength3(com), m3dGetVectorLength3(vtot));
	}
	i++;

	nb_integrate(world, DT);

	for (int i = 0; i < world->nBodies; i++) {
		nb_calculatePercievedForces(world, i);
		nb_calculateNormals(world, i);
		renderModel(world, i);
	}

	// Flush drawing commands
	glutSwapBuffers();
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

void usage(void) {
	int i;
	printf("Usage: nbtest <world>\n");
	printf(" where <world> is one of: ");
	for (i = 0; i < nCreators - 1; i++) {
		printf("%s ", creators[i].name);
	}
	printf("%s\n", creators[i].name);
}

int main(int argc, char* argv[]) {
	if (argc != 2) {
		usage();
		return -1;
	}
	
	for (int i = 0; i < nCreators; i++) {
		if (strcmp(argv[1], creators[i].name) == 0)
			world = (creators[i].creator)();
	}

	if (world == NULL) {
		printf("You need to specify a world\n");
		usage();
		return -1;
	}

	float mtot;
	M3DVector3f vtot, com;
	nb_getSummaryValues(mtot, com, vtot, initialEnergy, world);
	
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(800, 600);
	glutCreateWindow("NBody Physics Test");
	glutReshapeFunc(ChangeSize);
	glutDisplayFunc(RenderScene);

	SetupRC();

	glutMainLoop();
	return 0;
}
