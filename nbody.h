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
} nb_pva_t;

typedef struct nb_body {
	float mass;
	float radius;

	nb_pva_t *pva;

	sm_model     *unitSphere;
	M3DVector3f  *sampleVertices;
	M3DVector3f  *perceivedForceAtSample;
	float        *pfNormalComponent;
	M3DVector3f  *displayVertices;
	M3DVector3f  *displayNormals;
} nb_body_t;

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
	nb_body_t *bodies;
	
	nb_pva_t *getCurrentPVA(int body) { return &bodies[body].pva[current()]; }
} nb_world_t;

typedef struct nb_creator {
	const char *name;
	nb_world_t *(*creator)(void);
} nb_creator_t;

extern int nCreators;
extern nb_creator_t creators[];

nb_world_t * nb_createWorld(int nBodies);
void nb_freeWorld(nb_world_t *world);

void nb_calculateForceFieldAt(M3DVector3f ff, M3DVector3f pos, nb_world_t world, int excludeBody);
void nb_integrate(nb_world_t *world, float dt);

void nb_calculatePercievedForces(nb_world_t *world, int body);
void nb_calculateNormals(nb_world_t *world, int body);

void nb_getSummaryValues(float &totalMass, M3DVector3f centerOfMass, M3DVector3f totalVelocity, float &totalEnergy, nb_world_t* world);

#endif /* _N_BODY_H */
