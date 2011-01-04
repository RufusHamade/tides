
#include <GL/gl.h>
#include <stdlib.h>
#include <stdio.h>

#include "sphereModels.h"

#ifdef DEBUG
# define DBGPRINT(...) printf(__VA_ARGS__)
# define DBGPRINTMODEL(m) printModel(m)
#else
# define DBGPRINT(...)
# define DBGPRINTMODEL(m)
#endif

#define phi 1.618033989	 /* (1 + sqrt(5)) / 2) */
#define r   1.902113033   /* sqrt(1^2 + phi^2)  */

/* icosahedron vertices are:
 * ( +-1/r,  +-phi/r, 0)
 * (0,       +-1/r,   +-phi/r)
 * (+-phi/r, 0,       +-1/r)
 * 
 * We consider (1/r, phi/r, 0) top of icosahedron.
 * Next 5 vertices are adjacent to top.
 * Next 5 vertices are adjacent to bottom.
 * last vertex is bottom of icosahedron (-1/r, -phi/r, 0). */
static M3DVector3f icosahedron[12] = 
	{ { 1.0/r,  phi/r,  0},

      {-1.0/r,  phi/r,  0},
      { 0,      1.0/r,  phi/r},
      { phi/r,  0,      1.0/r},
      { phi/r,  0,     -1.0/r},
      { 0,      1.0/r,  -phi/r},

      { 0,     -1.0/r, -phi/r},
      {-phi/r,  0,     -1.0/r},
      {-phi/r,  0,      1.0/r},
      { 0,     -1.0/r,  phi/r},
      { 1.0/r, -phi/r,  0},

      {-1.0/r, -phi/r,  0}      };

/* We can draw the isocahedron as 2 fans and a ribbon. */
int ico_top[]    = {0, 1, 2, 3, 4, 5, 1};
int ico_bottom[] = {11, 10, 6, 7, 8, 9, 10};
int ico_middle[] = {1, 8, 2, 9, 3, 10, 4, 6, 5, 7, 1, 8};

static sm_model *createModel(int nVertices, int nLayers, int nChunks) {
	sm_model *newModel = (sm_model*)malloc(sizeof(sm_model));
	newModel->nVertices = nVertices;
	newModel->nLayers   = nLayers;
	newModel->nChunks   = nChunks;
	newModel->vertices  = (M3DVector3f*)malloc(nVertices*sizeof(M3DVector3f));
	newModel->levels    = (sm_level*)calloc((nLayers + 1), sizeof(sm_level));
	newModel->layers    = (sm_layer*)calloc(nLayers, sizeof(sm_layer));
	newModel->chunks    = (sm_chunk*)calloc(nChunks, sizeof(sm_chunk));
	return newModel;
}

void sm_freeModel(sm_model *m) {
    int i;
    for (i = 0; i < m->nChunks; i++) {
        free(m->chunks[i].nodes);
    }
    free(m->chunks);
    free(m->levels);
    free(m->layers);
    free(m->vertices);
    free(m);
}

void sm_renderIcosahedronFrame() {
	sm_model *m = sm_getUnitIsocahedron();
	int j;

    glColor3f(1.0f, 0.0f, 0.0f);
    int pnl[] = {0, 1, 10, 11};
    glBegin(GL_TRIANGLE_STRIP);
    for (j = 0; j < 4; j++)
        glVertex3fv(m->vertices[pnl[j]]);
    glEnd();

    glColor3f(0.0f, 1.0f, 0.0f);
    int pnl2[] = {2, 5, 9, 6};
    glBegin(GL_TRIANGLE_STRIP);
    for (j = 0; j < 4; j++)
        glVertex3fv(m->vertices[pnl2[j]]);
    glEnd();

    glColor3f(0.0f, 0.0f, 1.0f);
    int pnl3[] = {3, 4, 8, 7};
    glBegin(GL_TRIANGLE_STRIP);
    for (j = 0; j < 4; j++)
        glVertex3fv(m->vertices[pnl3[j]]);
    glEnd();
    
    sm_freeModel(m);
}	

void sm_renderModel(sm_model *m) {
    int i, j;

    for (i = 0; i < m->nChunks; i++) {
        sm_chunk* c = &m->chunks[i];

        glBegin(c->type);
        for (j = 0; j < c->nNodes; j++)
            glVertex3fv(m->vertices[c->nodes[j]]);
        glEnd();
    }    
}

#ifdef DEBUG
static void printModel(sm_model *m) {
	printf("Vertexes %d, layers %d, chunks %d\n", m->nVertices, m->nLayers, m->nChunks);
	int i;
    for (i = 0; i < m->nLayers; i++) {
		sm_level *lv = &m->levels[i];
	    printf(" Layer %d: Vertexes: %d - %d\n", i, lv->firstVertex, lv->nVertices);
	    sm_layer *la = &m->layers[i];
	    printf(" Level %d: Chunks: %d - %d\n", i, la->firstChunk, la->nChunks);
	}
	sm_level *lv = &m->levels[i];
    printf(" Layer %d: Vertexes: %d - %d\n", i, lv->firstVertex, lv->nVertices);
};
#endif

static inline void calculateBisector(M3DVector3f mid, M3DVector3f v1, M3DVector3f v2) {
	mid[0] = v1[0] + v2[0];
	mid[1] = v1[1] + v2[1];
	mid[2] = v1[2] + v2[2];
	
	float abs = sqrt(mid[0]*mid[0] + mid[1]*mid[1] + mid[2]*mid[2]);

	mid[0] = mid[0]/abs;
	mid[1] = mid[1]/abs;
	mid[2] = mid[2]/abs;
}

// We create a new level from an old level by doubling the number of vertices in that level.
// The two levels always have the same origin.
// We use this fact to calculate the index of a vertex in the new model given the index of the vertex in the old model.
static inline int getNewNodeFromOld(sm_level *oldL, sm_level *newL, int oldNode) {
	int off = oldNode - oldL->firstVertex;
	return newL->firstVertex + 2*off;
}

// Increment an index within a given level.  When we reach the end of the level, wrap to the beginning.
static inline void incIdx(int &idx, int step, sm_level *l) {
	idx += step;
	if (idx >= l->firstVertex + l->nVertices)
		idx -= l->nVertices;
}

// Given 2 levels, a starting point in each level, and a number of nodes
// Create an appropriately sized chunk of the layer joining those two levels
static inline void createStripFromLevels(sm_chunk *chunk, int nNodes, sm_level *l1, sm_level *l2, int s1, int s2) {
	chunk->type   = GL_TRIANGLE_STRIP;
	chunk->nNodes = nNodes;
	chunk->nodes  = (int*)malloc(nNodes * sizeof(int));
	
	DBGPRINT("  Creating strip of %d nodes from levels starting %d %d\n", nNodes, s1, s2);
	for (int k = 0; k < nNodes; k++) {
		if (k%2 == 0) {
			chunk->nodes[k] = s1; 
			incIdx(s1, 1, l1);
		}
		else {
			chunk->nodes[k] = s2; 
			incIdx(s2, 1, l2);
		}

		DBGPRINT("   adding vertex %d\n", chunk->nodes[k]);
	}
}

// Create a new, larger model suitable for containing the more accurate version of the given old model
// Allocate the levels, layers and chunks at the same time, and assign vertices and chunks to levels and layers respectively.
// New vertices are interpolated and assigned to chunks elsewhere.
static sm_model *createNewModelFromOld(sm_model *oldModel) {
	int i;

	int nTris = 0;
	int nChunks = 0;

	for (i = 0; i < oldModel->nChunks; i++) {
		nTris += oldModel->chunks[i].nNodes - 2;
		if (oldModel->chunks[i].type == GL_TRIANGLE_FAN)        // Each fan becomes a fan + n strips where n is the number of tris;
			nChunks += 1 + oldModel->chunks[i].nNodes - 2;
		else if (oldModel->chunks[i].type == GL_TRIANGLE_STRIP)	// Each strip becomes 2 strips.
			nChunks += 2;
	}

	// Every edge in the old model aquires a new vertex.  
	// There are 3 edges per tri, but each edge is shared  by 2 tris.
	sm_model *newModel = createModel(oldModel->nVertices + nTris * 3 / 2, oldModel->nLayers*2, nChunks);

	DBGPRINT("Create new from old (%d tris, %d vertices, %d layers, %d chunks) -> new (%d vertices, %d layers, %d chunks)\n", nTris, 
			oldModel->nVertices, oldModel->nLayers, oldModel->nChunks, newModel->nVertices, newModel->nLayers, newModel->nChunks);

	newModel->levels[0].firstVertex = 0;
	newModel->levels[0].nVertices   = 1;

	newModel->layers[0].firstChunk = 0;
	newModel->layers[0].nChunks    = 1;

	newModel->levels[1].firstVertex = newModel->levels[0].followingVertex();
	newModel->levels[1].nVertices   = oldModel->chunks[0].nNodes - 2;

	newModel->layers[1].firstChunk = newModel->layers[0].followingChunk();
	newModel->layers[1].nChunks    = oldModel->chunks[0].nNodes - 2;

	for (i = 1; i < oldModel->nLayers - 1; i++) {
		newModel->levels[2*i].firstVertex = newModel->levels[2*i - 1].followingVertex();
		newModel->levels[2*i].nVertices   = oldModel->levels[i].nVertices * 2;

		newModel->layers[2*i].firstChunk = newModel->layers[2*i - 1].followingChunk();
		newModel->layers[2*i].nChunks    = oldModel->layers[i].nChunks;

		newModel->levels[2*i + 1].firstVertex = newModel->levels[2*i].followingVertex();
		newModel->levels[2*i + 1].nVertices   = oldModel->levels[i].nVertices + oldModel->levels[i+1].nVertices;

		newModel->layers[2*i + 1].firstChunk = newModel->layers[2*i].followingChunk();
		newModel->layers[2*i + 1].nChunks    = oldModel->layers[i].nChunks;
	}

	newModel->levels[2*i].firstVertex = newModel->levels[2*i - 1].followingVertex();
	newModel->levels[2*i].nVertices   = oldModel->levels[i].nVertices * 2;
	
	newModel->layers[2*i].firstChunk = newModel->layers[2*i - 1].followingChunk();
	newModel->layers[2*i].nChunks    = oldModel->chunks[oldModel->nChunks - 1].nNodes - 2;

	newModel->levels[2*i + 1].firstVertex = newModel->levels[2*i].followingVertex();
	newModel->levels[2*i + 1].nVertices   = oldModel->chunks[oldModel->nChunks - 1].nNodes - 2;
	
	newModel->layers[2*i + 1].firstChunk = newModel->layers[2*i].followingChunk();
	newModel->layers[2*i + 1].nChunks    = 1;
	
	newModel->levels[2*i + 2].firstVertex = newModel->levels[2*i + 1].followingVertex();
	newModel->levels[2*i + 2].nVertices   = 1;

	return newModel;
}

// Create a new, more accurate level by creating vertices bisecting each vertex in the old level.
static void subdivideLevel(sm_model *newM, int newLIdx, sm_model *oldM, int oldLIdx) {
	sm_level *newL = &newM->levels[newLIdx];
	sm_level *oldL = &oldM->levels[oldLIdx];

	DBGPRINT(" Subdividing level %d (%d %d) to get level %d (%d %d)\n", 
	         oldLIdx, oldL->firstVertex, oldL->nVertices, 
	         newLIdx, newL->firstVertex, newL->nVertices);
	int i, oldV, newV;
	for (i = 0, oldV = oldL->firstVertex, newV = newL->firstVertex; 
		 i < oldL->nVertices - 1; 
		 i++, oldV++, newV+=2) {
		m3dCopyVector3(newM->vertices[newV], oldM->vertices[oldV]);
		calculateBisector(newM->vertices[newV + 1], oldM->vertices[oldV], oldM->vertices[oldV+1]); 
	}
	m3dCopyVector3(newM->vertices[newV], oldM->vertices[oldV]);
	calculateBisector(newM->vertices[newV + 1], oldM->vertices[oldV], oldM->vertices[oldL->firstVertex]);
}

// The chunks tell us which vertices are adjoining in each layer
// Use this fact to create a new vertex between each neighbouring node on the two levels bordering this layer 
static void createNewLevel(sm_model *newM, int newLIdx, sm_model *oldM, int oldLIdx) {
	sm_level *newL = &newM->levels[newLIdx];
	sm_layer *oldL = &oldM->layers[oldLIdx];

	DBGPRINT(" Processing chunks in layer %d to get level %d (%d %d)\n", 
	         oldLIdx, newLIdx, newL->firstVertex, newL->nVertices);
	int newV = newL->firstVertex;
	for (int i = 0; i < oldL->nChunks; i++) {
		DBGPRINT("  Processing chunk %d\n", oldL->firstChunk + i);
		sm_chunk *c = &oldM->chunks[oldL->firstChunk + i];
		
		for (int j = 0; j < c->nNodes - 2; j++, newV++) {
			calculateBisector(newM->vertices[newV], oldM->vertices[c->nodes[j]], oldM->vertices[c->nodes[j + 1]]); 
		}
	}
}

static void recreateFan(sm_model *newModel, int newLApex, int newLNext, int newLLast, 
						sm_model *oldModel, int oldLApex, int oldLNext) {
	DBGPRINT(" Converting fan from layers (%d %d) to get layers (%d %d %d)\n", 
	         oldLApex, oldLNext, newLApex, newLNext, newLLast);
	int newApex = newModel->levels[newLApex].firstVertex;
	int oldApex = oldModel->levels[oldLApex].firstVertex;
	m3dCopyVector3(newModel->vertices[newApex], oldModel->vertices[oldApex]);

	int fanChunkIdx = newModel->layers[(newLApex < newLNext) ? newLApex : newLNext].firstChunk;  
	sm_chunk *fanChunk = &newModel->chunks[fanChunkIdx];
	fanChunk->type   = GL_TRIANGLE_FAN;
	fanChunk->nNodes = 2 + newModel->levels[newLNext].nVertices;
    fanChunk->nodes  = (int*)malloc(fanChunk->nNodes * sizeof(int));

	fanChunk->nodes[0] = newApex;
	int i, oldV, newV, newV2;
	int newChunkIdx = newModel->layers[(newLNext < newLLast) ? newLNext : newLLast].firstChunk;

	for (i = 0, oldV = oldModel->levels[oldLNext].firstVertex, 
		 newV  = newModel->levels[newLNext].firstVertex,
		 newV2 = newModel->levels[newLLast].firstVertex; 
		 i < oldModel->levels[oldLNext].nVertices;
		 i++, oldV++, newV++, newV2+=2, newChunkIdx++) {
		calculateBisector(newModel->vertices[newV], oldModel->vertices[oldApex], oldModel->vertices[oldV]);
		fanChunk->nodes[i+1] = newV;
		createStripFromLevels(&newModel->chunks[newChunkIdx], 5, 
							  &newModel->levels[newLLast], &newModel->levels[newLNext],
							  newV2, newV);
	}

	fanChunk->nodes[i+1] = fanChunk->nodes[1];
}

static void recreateLayer(sm_model *newModel, int newLayer0, int newLayer1, int newL0, int newL1, int newL2,  
						  sm_model *oldModel, int oldLayer, int oldL0, int oldL1) {
	DBGPRINT(" Processing layer %d (%d %d) to get layers %d %d (%d %d %d)\n", 
	         oldLayer, oldL0, oldL1, newLayer0, newLayer1, newL0, newL1, newL2);
	
	int nv1 = newModel->levels[newL1].firstVertex;

	for (int i = 0; i < oldModel->layers[oldLayer].nChunks; i++) {
		sm_chunk *oldChunk  = oldModel->getChunk(oldLayer,  i);
		sm_chunk *newChunk0 = newModel->getChunk(newLayer0, i);
		sm_chunk *newChunk1 = newModel->getChunk(newLayer1, i);
		int ov0 = oldChunk->nodes[0];
		int ov1 = oldChunk->nodes[1];
		int nv0 = getNewNodeFromOld(&oldModel->levels[oldL0], &newModel->levels[newL0], ov0); 
		int nv2 = getNewNodeFromOld(&oldModel->levels[oldL1], &newModel->levels[newL2], ov1);

		int nNodes = (oldChunk->nNodes - 2) * 2 + 2; 

		if (oldChunk->nNodes % 2 == 0) {
			createStripFromLevels(newChunk0, nNodes, 
								  &newModel->levels[newL0], &newModel->levels[newL1],
								  nv0, nv1);
			createStripFromLevels(newChunk1, nNodes,
								  &newModel->levels[newL1], &newModel->levels[newL2],
								  nv1, nv2);
		}
		else {
			createStripFromLevels(newChunk0, nNodes + 1, 
								  &newModel->levels[newL0], &newModel->levels[newL1],
								  nv0, nv1);
			createStripFromLevels(newChunk1, nNodes - 1,
								  &newModel->levels[newL1], &newModel->levels[newL2],
								  nv1, nv2);
		}
		
		incIdx(nv1, oldChunk->nNodes - 2, &newModel->levels[newL1]);
	}
}

sm_model *sm_getUnitIsocahedron() {
	//DBGPRINT("Calculating isocahedron\n");
    sm_model *ret = createModel(sizeof(icosahedron)/sizeof(M3DVector3f), 3, 3);
    memcpy(ret->vertices, icosahedron, sizeof(icosahedron));

	int i = 0;

    ret->chunks[i].type   = GL_TRIANGLE_FAN;
    ret->chunks[i].nNodes = sizeof(ico_top)/sizeof(int);
    ret->chunks[i].nodes  = (int*)malloc(sizeof(ico_top));
    ret->levels[i].firstVertex = 0;
    ret->levels[i].nVertices   = 1;
    ret->layers[i].firstChunk  = i;
    ret->layers[i].nChunks     = 1;
    memcpy(ret->chunks[i].nodes, ico_top, sizeof(ico_top));
    i++;
    
    ret->chunks[i].type = GL_TRIANGLE_STRIP;
    ret->chunks[i].nNodes = sizeof(ico_middle)/sizeof(int);
    ret->chunks[i].nodes = (int*)malloc(sizeof(ico_middle));
    ret->levels[i].firstVertex = ret->levels[i-1].followingVertex();
    ret->levels[i].nVertices   = 5;
    ret->layers[i].firstChunk  = i;
    ret->layers[i].nChunks     = 1;
    memcpy(ret->chunks[i].nodes, ico_middle, sizeof(ico_middle));
	i++;

    ret->chunks[i].type = GL_TRIANGLE_FAN;
    ret->chunks[i].nNodes = sizeof(ico_bottom)/sizeof(int);
    ret->chunks[i].nodes = (int*)malloc(sizeof(ico_bottom));
    ret->levels[i].firstVertex = ret->levels[i-1].followingVertex();
    ret->levels[i].nVertices   = 5;
    ret->layers[i].firstChunk  = i;
    ret->layers[i].nChunks     = 1;
    memcpy(ret->chunks[i].nodes, ico_bottom, sizeof(ico_bottom));    
	i++;

    ret->levels[i].firstVertex = ret->levels[i-1].followingVertex();
    ret->levels[i].nVertices   = 1;

	ret->nChunks = i;
	ret->nLayers = i;

	DBGPRINTMODEL(ret);
    return ret;
}

sm_model *sm_getUnitSphere(int precision) {
	if (precision == 0)
		return sm_getUnitIsocahedron();

	sm_model *oldModel = sm_getUnitSphere(precision - 1);

	DBGPRINT("Calculating sphere precision %d\n", precision);

	sm_model *newModel = createNewModelFromOld(oldModel);
	int i;

	recreateFan(newModel, 0, 1, 2, oldModel, 0, 1);
	for (i = 1; i < oldModel->nLayers - 1; i++) {
		subdivideLevel(newModel, 2*i, oldModel, i);
		createNewLevel(newModel, 2*i+1, oldModel, i);

		sm_layer *oldLayer = &oldModel->layers[i];
		sm_chunk *oldChunk = &oldModel->chunks[oldLayer->firstChunk];
		if (oldChunk->nodes[0] < oldChunk->nodes[1])
			recreateLayer(newModel, 2*i, 2*i + 1, 2*i, 2*i + 1, 2*i + 2, oldModel, i, i, i + 1);
		else
			recreateLayer(newModel, 2*i + 1, 2*i, 2*i + 2, 2*i + 1, 2*i, oldModel, i, i + 1, i);
	}
	subdivideLevel(newModel, 2*i, oldModel, i);

	recreateFan(newModel, newModel->nLayers, newModel->nLayers - 1, newModel->nLayers - 2, 
	            oldModel, oldModel->nLayers, oldModel->nLayers - 1);

	DBGPRINTMODEL(newModel);
	sm_freeModel(oldModel);
	return newModel;
}
