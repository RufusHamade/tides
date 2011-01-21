/* Data structures used for n-body problem. */

#ifndef _N_BODY_H
#define _N_BODY_H

#include "math3d.h"
#include "sphereModels.h"

#define BIGG 1

typedef struct nb_pva {
	M3DVector3f  position;
	M3DVector3f  velocity;
	M3DVector3f  acceleration;
} nb_pva;

typedef struct nb_body {
	float mass;
	float radius;

	nb_pva *pva;

	sm_model     *unitSphere;
	M3DVector3f  *sampleVertices;
	M3DVector3f  *perceivedForceAtSample;
	float        *pfNormalComponent;
	M3DVector3f  *displayVertices;
	M3DVector3f  *displayNormals;
} nb_body;

typedef struct nb_world {
	float radius;
	float stiffness;
	float bounceFudgeFactor;
	
	float t;
	int slot;
	int slotMax;
	
	int current()         { return slot; }
	int next()            { return (slot != slotMax) ? slot + 1 : 0; }
	int prev()            { return (slot != 0)       ? slot - 1 : slotMax; }
	void inc(float dt)    { slot = next(); t += dt;}	

	int nBodies;
	nb_body *bodies;
	
	nb_pva *getCurrentPVA(int body) { return &bodies[body].pva[current()]; }
} nb_world;

nb_world * nb_createWorld(int nBodies);
void nb_freeWorld(nb_world *world);

nb_world *nb_createOrbit2World();
nb_world *nb_createOrbit3World();
nb_world *nb_createBounce2World();
nb_world *nb_createBounce2bWorld();
nb_world *nb_createBounce4World();
nb_world *nb_createBounce9World();

void nb_calculateForceFieldAt(M3DVector3f ff, M3DVector3f pos, nb_world world, int excludeBody);
void nb_integrate(nb_world *world, float dt);

void nb_calculatePercievedForces(nb_world *world, int body);
void nb_calculateNormals(nb_world *world, int body);

void nb_getSummaryValues(float &totalMass, M3DVector3f centerOfMass, M3DVector3f totalVelocity, float &totalEnergy, nb_world* world);

#endif /* _N_BODY_H */
