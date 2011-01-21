
#include <stdlib.h>
#include <stdio.h>

#include "nbody.h"

const float DEFAULT_STIFFNESS = 0.1f;

// Make sure objects don't walk off the screen by transforming the model to center-of-mass coordinates
void centerModel(nb_world_t *world) {
	M3DVector3f vtot, com;
	float mtot, etot;
	
	nb_getSummaryValues(mtot, com, vtot, etot, world);

	for (int i = 0; i < world->nBodies; i++) {
		m3dSubtractVectors3(world->getCurrentPVA(i)->velocity, world->getCurrentPVA(i)->velocity, vtot);
		m3dSubtractVectors3(world->getCurrentPVA(i)->position, world->getCurrentPVA(i)->position, com);
	}
}

nb_world_t *nb_createOrbit2World() {
	nb_world_t *world = nb_createWorld(2);
	world->radius = 20.0f;
	world->stiffness = DEFAULT_STIFFNESS;
	world->bounceFudgeFactor = 1.5f;

	world->bodies[0].mass   = 10.0f;
	world->bodies[0].radius = 2.5f;
	m3dLoadVector3(world->bodies[0].pva[world->current()].position, 0.0f, 0.0f, 0.0f);
	m3dLoadVector3(world->bodies[0].pva[world->current()].velocity, 0.0f, 0.0f, 0.0f);

	world->bodies[1].mass   = 1.0f;
	world->bodies[1].radius = 0.5f;
	m3dLoadVector3(world->bodies[1].pva[world->current()].position, -10.0f, 0.0f, 0.0f);
	m3dLoadVector3(world->bodies[1].pva[world->current()].velocity, 0.0f, 0.7f, 0.7f);
	
	centerModel(world);
	return world;
}

nb_world_t *nb_createOrbit3World() {
	nb_world_t *world = nb_createWorld(3);
	world->radius = 20.0f;
	world->stiffness = DEFAULT_STIFFNESS;
	world->bounceFudgeFactor = 3.0f;

	world->bodies[0].mass   = 1.0f;
	world->bodies[0].radius = 1.5f;
	m3dLoadVector3(world->bodies[0].pva[world->current()].position, 0.0f, 0.0f, 0.0f);
	m3dLoadVector3(world->bodies[0].pva[world->current()].velocity, 0.0f, 0.0f, 0.0f);

	world->bodies[1].mass   = 0.1f;
	world->bodies[1].radius = 0.3f;
	m3dLoadVector3(world->bodies[1].pva[world->current()].position, -10.0f, 0.0f, 0.0f);
	m3dLoadVector3(world->bodies[1].pva[world->current()].velocity, 0.0f, 0.2f, 0.25f);

	world->bodies[2].mass   = 0.02f;
	world->bodies[2].radius = 0.05f;
	m3dLoadVector3(world->bodies[2].pva[world->current()].position, -11.0f, 0.0f, 0.0f);
	m3dLoadVector3(world->bodies[2].pva[world->current()].velocity, 0.0f, 0.45f, 0.0f);
	
	centerModel(world);
	return world;
}

nb_world_t *nb_createBounce2World() {
	nb_world_t *world = nb_createWorld(2);
	world->radius = 20.0f;
	world->stiffness = DEFAULT_STIFFNESS * 5;
	world->bounceFudgeFactor = 1.5f;

	world->bodies[0].mass   = 1.0f;
	world->bodies[0].radius = 1.0f;
	m3dLoadVector3(world->bodies[0].pva[world->current()].position, 0.0f, 0.0f, 0.0f);
	m3dLoadVector3(world->bodies[0].pva[world->current()].velocity, 0.0f, 0.0f, 0.0f);

	world->bodies[1].mass   = 2.0f;
	world->bodies[1].radius = 2.0f;
	m3dLoadVector3(world->bodies[1].pva[world->current()].position, -10.0f, 0.0f, 0.0f);
	m3dLoadVector3(world->bodies[1].pva[world->current()].velocity, 0.0f, 0.3f, 0.0f);
	
	centerModel(world);
	return world;
}

nb_world_t *nb_createBounce2bWorld() {
	nb_world_t *world = nb_createWorld(2);
	world->radius = 15.0f;
	world->stiffness = DEFAULT_STIFFNESS * 5;
	world->bounceFudgeFactor = 1.5f;

	world->bodies[0].mass   = 1.0f;
	world->bodies[0].radius = 1.0f;
	m3dLoadVector3(world->bodies[0].pva[world->current()].position, 0.0f, 0.0f, 0.0f);
	m3dLoadVector3(world->bodies[0].pva[world->current()].velocity, 0.0f, 0.0f, 0.0f);

	world->bodies[1].mass   = 2.0f;
	world->bodies[1].radius = 2.0f;
	m3dLoadVector3(world->bodies[1].pva[world->current()].position, -10.0f, 0.0f, 0.0f);
	m3dLoadVector3(world->bodies[1].pva[world->current()].velocity, 1.0f, 0.3f, 0.0f);
	
	centerModel(world);
	return world;
}

nb_world_t *nb_createBounce4World() {
	nb_world_t *world = nb_createWorld(4);
	world->radius = 20.0f;
	world->stiffness = DEFAULT_STIFFNESS * 5;
	world->bounceFudgeFactor = 1.5f;

	world->bodies[0].mass   = 1.0f;
	world->bodies[0].radius = 1.0f;
	m3dLoadVector3(world->bodies[0].pva[world->current()].position, 10.0f, 10.0f, 0.0f);
	m3dLoadVector3(world->bodies[0].pva[world->current()].velocity, 0.0f, 0.0f, 0.0f);

	world->bodies[1].mass   = 1.0f;
	world->bodies[1].radius = 1.0f;
	m3dLoadVector3(world->bodies[1].pva[world->current()].position, 10.0f, -10.0f, 0.0f);
	m3dLoadVector3(world->bodies[1].pva[world->current()].velocity, 0.0f, 0.0f, 0.0f);

	world->bodies[2].mass   = 1.0f;
	world->bodies[2].radius = 1.0f;
	m3dLoadVector3(world->bodies[2].pva[world->current()].position, -10.0f, -10.0f, 0.0f);
	m3dLoadVector3(world->bodies[2].pva[world->current()].velocity, 0.0f, 0.0f, 0.0f);

	world->bodies[3].mass   = 1.0f;
	world->bodies[3].radius = 1.0f;
	m3dLoadVector3(world->bodies[3].pva[world->current()].position, -10.0f, 10.0f, 0.0f);
	m3dLoadVector3(world->bodies[3].pva[world->current()].velocity, 0.0f, 0.0f, 0.0f);
	
	centerModel(world);
	return world;
}

nb_world_t *nb_createBounce9World() {
	nb_world_t *world = nb_createWorld(9);
	world->radius = 15.0f;
	world->stiffness = DEFAULT_STIFFNESS * 5;
	world->bounceFudgeFactor = 1.5f;
	
	for (int i = 0; i < 9; i++) {
		int x = i%3 - 1;
		int y = i/3 - 1;
		world->bodies[i].mass   = 1.0f;
		world->bodies[i].radius = 1.0f;
		m3dLoadVector3(world->bodies[i].pva[world->current()].position, 5.0f*x, 5.0f*y, 0.0f);
		m3dLoadVector3(world->bodies[i].pva[world->current()].velocity, 0.0f, 1.0f*x, 1.0f*y);
	}
	
	centerModel(world);
	return world;
}

nb_creator_t creators[] = {
	{"orbit2",    nb_createOrbit2World},
	{"orbit3",    nb_createOrbit3World},
	{"bounce2",   nb_createBounce2World},
	{"bounce2b",  nb_createBounce2bWorld},
	{"bounce4",   nb_createBounce4World},
	{"bounce9",   nb_createBounce9World}
};

int nCreators = sizeof(creators)/sizeof(nb_creator_t);

