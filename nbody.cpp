
#include <stdlib.h>
#include <stdio.h>

#include "nbody.h"

#define NSLOTS 2

static inline void weightedAccumulate(M3DVector3f a, M3DVector3f v, float weight) {
	a[0] += v[0] * weight;
	a[1] += v[1] * weight;
	a[2] += v[2] * weight;
}

static void calculateForceFieldAt(M3DVector3f ff, M3DVector3f pos, nb_world *world, int slot, int excludeBody) {
	m3dLoadVector3(ff, 0.0f, 0.0f, 0.0f);

	for (int j = 0; j < world->nBodies; j++) {
		if (j == excludeBody)
			continue;

		M3DVector3f temp;
		m3dSubtractVectors3(temp, world->bodies[j].pva[slot].position, pos);
		float r = m3dGetVectorLength3(temp);
		float scale = BIGG*world->bodies[j].mass/(r*r*r);
		m3dScaleVector3(temp, scale);
		m3dAddVectors3(ff, ff, temp);
	}
}

void nb_calculateForceFieldAt(M3DVector3f ff, M3DVector3f pos, nb_world *world, int excludeBody) {
	calculateForceFieldAt(ff, pos, world, world->current(), excludeBody);
}


static inline void calculateAccelerations(nb_world *world, int slot) {
	for (int i = 0; i < world->nBodies; i++) {
		calculateForceFieldAt(world->bodies[i].pva[slot].acceleration, world->bodies[i].pva[slot].position, world, slot, i);
	}
}

static inline void integrateOneEuler(M3DVector3f f, M3DVector3f i, M3DVector3f ci, float dt) {
	f[0] = i[0] + ci[0]*dt;	
	f[1] = i[1] + ci[1]*dt; 
	f[2] = i[2] + ci[2]*dt;
}

static inline void integrateOneTrapezoid(M3DVector3f f, M3DVector3f i, M3DVector3f ci, M3DVector3f cf, float dt) {
	f[0] = i[0] + (ci[0] + cf[0])/2.0f * dt;	
	f[1] = i[1] + (ci[1] + cf[1])/2.0f * dt; 
	f[2] = i[2] + (ci[2] + cf[2])/2.0f * dt;
}

static inline void integrateEuler(nb_world *world, float dt, int from, int to) {
	calculateAccelerations(world, from);
	
	for (int i = 0; i < world->nBodies; i++) {
		integrateOneEuler(world->bodies[i].pva[to].velocity, world->bodies[i].pva[from].velocity, world->bodies[i].pva[from].acceleration, dt);
		integrateOneEuler(world->bodies[i].pva[to].position, world->bodies[i].pva[from].position, world->bodies[i].pva[from].velocity,     dt);
	}
}

static inline void reintegrateTrapezoid(nb_world *world, float dt, int from, int to) {
	calculateAccelerations(world, to);
	
	for (int i = 0; i < world->nBodies; i++) {
		integrateOneTrapezoid(world->bodies[i].pva[to].velocity, world->bodies[i].pva[from].velocity, world->bodies[i].pva[from].acceleration, world->bodies[i].pva[to].acceleration, dt);
		integrateOneTrapezoid(world->bodies[i].pva[to].position, world->bodies[i].pva[from].position, world->bodies[i].pva[from].velocity,     world->bodies[i].pva[to].velocity,     dt);
	}
}

static inline void handleImpacts(nb_world *world, int slot) {
	for (int i = 0; i < world->nBodies; i++) {
		nb_pva *pva_i = &world->bodies[i].pva[slot];
		
		for (int j = i+1; j < world->nBodies; j++) {
			nb_pva *pva_j = &world->bodies[j].pva[slot];
			
			M3DVector3f sep;
			m3dSubtractVectors3(sep, pva_i->position, pva_j->position);
			float d2 =  m3dGetVectorLengthSquared3(sep);
			float dmin = (world->bodies[i].radius + world->bodies[j].radius);
			
			// Small fudge factor so bodies don't get too close.  It looks funny.
			if (d2/world->bounceFudgeFactor > dmin * dmin)
				continue;

			// Bodies are too close.  Reverse the component of their velocity parralel to the vector joining their centers. 
			M3DVector3f v_com;
			m3dLoadVector3(v_com,  0.0f, 0.0f, 0.0f);
			weightedAccumulate(v_com,  pva_i->velocity, world->bodies[i].mass);
			weightedAccumulate(v_com,  pva_j->velocity, world->bodies[j].mass);
			m3dScaleVector3(v_com, 1.0f/(world->bodies[i].mass + world->bodies[j].mass));
			
			M3DVector3f vrel_i, vrel_j;
			m3dSubtractVectors3(vrel_i, pva_i->velocity, v_com);
			m3dSubtractVectors3(vrel_j, pva_j->velocity, v_com);
			m3dNormalizeVector3(sep);
			
			//calculate normal component of relative velocity.
			float vnorm_i = m3dDotProduct3(sep, vrel_i);
			float vnorm_j = m3dDotProduct3(sep, vrel_j);

			// Ensure bodies are actually moving towards each other.
			if (m3dDotProduct3(sep, vrel_i) > 0)
				continue;
			
			weightedAccumulate(pva_i->velocity, sep, -2.0f*vnorm_i);
			weightedAccumulate(pva_j->velocity, sep, -2.0f*vnorm_j);
		}

		// Also make sure the body doesn't escape to infinity.
		float r2 = m3dGetVectorLengthSquared3(pva_i->position);
		
		if (r2 < world->radius * world->radius)
			continue;
		
		if (m3dDotProduct3(pva_i->position, pva_i->velocity) < 0)
			continue;
			
		// Again reverse outbound component of velocity
		M3DVector3f n;
		m3dCopyVector3(n, pva_i->position);
		m3dNormalizeVector3(n);
		float vnorm = m3dDotProduct3(n, pva_i->velocity);

		weightedAccumulate(pva_i->velocity, n, -2.0f*vnorm);
	}
}

#define STEPS 100
void nb_integrate(nb_world *world, float dt) {
	for (int i = 0; i < STEPS; i++) {
		handleImpacts(world, world->current());
		integrateEuler(world, dt/100, world->current(), world->next());
		reintegrateTrapezoid(world, dt/100, world->current(), world->next());
		world->inc(dt/100);
	}
}

void nb_getSummaryValues(float &mtot, M3DVector3f com, M3DVector3f vtot, float &etot, nb_world* world) {
	mtot = 0.0f;
	m3dLoadVector3(com,  0.0f, 0.0f, 0.0f);
	m3dLoadVector3(vtot, 0.0f, 0.0f, 0.0f);
	etot = 0.0f;
	
	for (int i = 0; i < world->nBodies; i++) {
		mtot += world->bodies[i].mass;

		weightedAccumulate(com,  world->getCurrentPVA(i)->position, world->bodies[i].mass);
		weightedAccumulate(vtot, world->getCurrentPVA(i)->velocity, world->bodies[i].mass);
		
		etot += 0.5f * world->bodies[i].mass * m3dGetVectorLengthSquared3(world->getCurrentPVA(i)->velocity);

		for (int j = i + 1; j < world->nBodies; j++) {
			M3DVector3f dp;
			m3dSubtractVectors3(dp, world->getCurrentPVA(i)->position, world->getCurrentPVA(j)->position);
			etot -= BIGG * world->bodies[i].mass * world->bodies[j].mass/m3dGetVectorLength3(dp);
		}
	}

	m3dScaleVector3(com, 1.0f/mtot);
	m3dScaleVector3(vtot, 1.0f/mtot);
}

// Make sure objects don't walk off the screen by transforming the model to center-of-mass coordinates
void centerModel(nb_world *world) {
	M3DVector3f vtot, com;
	float mtot, etot;
	
	nb_getSummaryValues(mtot, com, vtot, etot, world);

	for (int i = 0; i < world->nBodies; i++) {
		m3dSubtractVectors3(world->getCurrentPVA(i)->velocity, world->getCurrentPVA(i)->velocity, vtot);
		m3dSubtractVectors3(world->getCurrentPVA(i)->position, world->getCurrentPVA(i)->position, com);
	}
}

nb_world * nb_createWorld(int nBodies) {
	nb_world *world = (nb_world *)malloc(sizeof(nb_world));
	world->nBodies = nBodies;
	world->t       = 0.0f;
	world->slot    = 0;
	world->slotMax = NSLOTS - 1;

	world->bodies = (nb_body *)malloc(nBodies * sizeof(nb_body));
	for (int i = 0; i < world->nBodies; i++) {
		world->bodies[i].pva = (nb_pva *)calloc(NSLOTS, sizeof(nb_pva));
		// Initially we use the same unit sphere for all bodies.  Later we may use more accurate unit spheres for larger bodies.
		sm_model *s = world->bodies[i].unitSphere = sm_getUnitSphere(3);
		world->bodies[i].sampleVertices = (M3DVector3f *)calloc(s->nVertices, sizeof(M3DVector3f));
		world->bodies[i].perceivedForceAtSample = (M3DVector3f *)calloc(s->nVertices, sizeof(M3DVector3f));
		world->bodies[i].pfNormalComponent = (float *)calloc(s->nVertices, sizeof(float));
		world->bodies[i].displayVertices = (M3DVector3f *)calloc(s->nVertices, sizeof(M3DVector3f));
		world->bodies[i].displayNormals = (M3DVector3f *)calloc(s->nVertices, sizeof(M3DVector3f));
	}

	return world;
}

void nb_freeWorld(nb_world *world) {
	for (int i = 0; i < world->nBodies; i++) {
		sm_freeModel(world->bodies[i].unitSphere);
		free(world->bodies[i].pva);
		free(world->bodies[i].sampleVertices);
		free(world->bodies[i].perceivedForceAtSample);
		free(world->bodies[i].pfNormalComponent);
		free(world->bodies[i].displayVertices);
		free(world->bodies[i].displayNormals);
	}
	free(world->bodies);
	free(world);
}

const float DEFAULT_STIFFNESS = 0.1f;

nb_world *nb_createOrbit2World() {
	nb_world *world = nb_createWorld(2);
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

nb_world *nb_createOrbit3World() {
	nb_world *world = nb_createWorld(3);
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

nb_world *nb_createBounce2World() {
	nb_world *world = nb_createWorld(2);
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

nb_world *nb_createBounce2bWorld() {
	nb_world *world = nb_createWorld(2);
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

nb_world *nb_createBounce4World() {
	nb_world *world = nb_createWorld(4);
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

nb_world *nb_createBounce9World() {
	nb_world *world = nb_createWorld(9);
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

void nb_calculatePercievedForces(nb_world *world, int body) {
	nb_body *b = &world->bodies[body];
	sm_model *s = b->unitSphere;
	for (int i = 0; i < s->nVertices; i++) {
		nb_pva *pva = world->getCurrentPVA(body);
		M3DVector3f n;  // Normal.
		m3dCopyVector3(n, b->unitSphere->vertices[i]);
		// Need to rotate the vertex too, when we have object rotation.
		M3DVector3f d;  // Displacement of sample vertex from center of body.
		m3dCopyVector3(d, n);
		m3dScaleVector3(d, b->radius);
	
		m3dAddVectors3(b->sampleVertices[i], pva->position, d); // Percieved force sample position
		nb_calculateForceFieldAt(b->perceivedForceAtSample[i], b->sampleVertices[i], world, body);  // Force Field at that position
		m3dSubtractVectors3(b->perceivedForceAtSample[i], b->perceivedForceAtSample[i], pva->acceleration); // Percieved force at that position.  I.e., -(f - a)
		b->pfNormalComponent[i] = m3dDotProduct3(n, b->perceivedForceAtSample[i]); // Normal component of percieved force.
		
		m3dScaleVector3(d, 1.0 + b->pfNormalComponent[i]/world->stiffness);
		m3dAddVectors3(b->displayVertices[i], pva->position, d); // Vertex position after tidal stretching
	}
}

void nb_calculateNormals(nb_world *world, int body) {
	nb_body *b = &world->bodies[body];
	sm_model *s = b->unitSphere;
	
	// Calculate normal for every tri.
	M3DVector3f triNormals[s->nTris];
	for (int i = 0; i < s->nTris; i++) {
		int v0i = s->indices[3*i]; // Index of first tri vertex.
		int v1i = s->indices[3*i + 1];
		int v2i = s->indices[3*i + 2];
		M3DVector3f dv1, dv2;
		m3dSubtractVectors3(dv1, b->displayVertices[v1i], b->displayVertices[v0i]);
		m3dSubtractVectors3(dv2, b->displayVertices[v2i], b->displayVertices[v0i]);
		m3dCrossProduct3(triNormals[i], dv1, dv2);
		m3dNormalizeVector3(triNormals[i]);
	}
	
	// use average of tri normals to calculate normal of vertex.
	for (int i = 0; i < s->nVertices; i++) {
		m3dLoadVector3(b->displayNormals[i], 0.0f, 0.0f, 0.0f);
		sm_vertexInfo *vi = &s->vInfo[i];
		for (int j = 0; j < vi->nTris; j++) {
			m3dAddVectors3(b->displayNormals[i], b->displayNormals[i], triNormals[vi->tris[j]]);
		}
		m3dNormalizeVector3(b->displayNormals[i]);
	}
}
