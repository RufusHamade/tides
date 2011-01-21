
#include <GL/gl.h>
#include <stdlib.h>
#include <stdio.h>

#include "sphereModels.h"

//#define DEBUG
#ifdef DEBUG
# define DBGPRINT(...) printf(__VA_ARGS__)
# define DBGPRINTMODEL(m) printModel(m)
#else
# define DBGPRINT(...)
# define DBGPRINTMODEL(m)
#endif

static sm_model_t *createModel(int nVertices, int nLayers, int nChunks, int nTris) {
	sm_model_t *newModel = (sm_model_t*)calloc(1, sizeof(sm_model_t));
	newModel->nVertices = nVertices;
	
	newModel->nLayers   = nLayers;
	newModel->nChunks   = nChunks;
	newModel->nTris     = nTris;
	newModel->nextTri   = 0;
	newModel->vertices  = (M3DVector3f*)calloc(nVertices, sizeof(M3DVector3f));
	newModel->vInfo     = (sm_vertexInfo_t*)calloc(nVertices, sizeof(sm_vertexInfo_t));
	newModel->levels    = (sm_level_t*)calloc((nLayers + 1), sizeof(sm_level_t));
	newModel->layers    = (sm_layer_t*)calloc(nLayers, sizeof(sm_layer_t));
	newModel->chunks    = (sm_chunk_t*)calloc(nChunks, sizeof(sm_chunk_t));
	newModel->tris      = (sm_tri_t*)calloc(nTris, sizeof(sm_tri_t));
	newModel->indices   = (unsigned *)calloc(nTris * 3, sizeof(unsigned));
	return newModel;
}

void sm_freeModel(sm_model_t *m) {
    int i;
    for (i = 0; i < m->nChunks; i++) {
        free(m->chunks[i].vertices);
    }
    free(m->chunks);
    free(m->levels);
    free(m->layers);
    free(m->vertices);
    free(m->vInfo);
    free(m->tris);
    free(m->indices);
    free(m);
}

void sm_renderChunk(sm_model_t *m, sm_chunk_t *c) {
	int start = 0;

    glBegin(c->type);
    if (c->type == GL_TRIANGLE_FAN) {
		glNormal3fv(m->vertices[c->vertices[0]]);
		glVertex3fv(m->vertices[c->vertices[0]]);
		start++;
	}
	if (c->backwards) {
		for (int j = c->nVertices - 1; j >= start; j--) {
			glNormal3fv(m->vertices[c->vertices[j]]);
			glVertex3fv(m->vertices[c->vertices[j]]);
		}
	}
	else {
		for (int j = start; j < c->nVertices; j++) {
			glNormal3fv(m->vertices[c->vertices[j]]);
			glVertex3fv(m->vertices[c->vertices[j]]);
		}
	}
    glEnd();
}


#ifdef DEBUG
static void printModel(sm_model_t *m) {
	printf("Vertexes %d, layers %d, chunks %d, tris %d (%d)\n", m->nVertices, m->nLayers, m->nChunks, m->nTris, m->nextTri);
	int i;
    for (i = 0; i < m->nLayers; i++) {
		sm_level_t *lv = &m->levels[i];
	    printf(" Layer %d: Vertexes: %d - %d\n", i, lv->firstVertex, lv->nVertices);
	    sm_layer_t *la = &m->layers[i];
	    printf(" Level %d: Chunks: %d - %d\n", i, la->firstChunk, la->nChunks);
		for (int j = 0; j < la->nChunks; j++) {
			sm_chunk_t *c = &m->chunks[la->firstChunk + j];
			printf("  Chunk %d %d %d:", c->type, c->nVertices, c->backwards);
			for (int k = 0; k < c->nVertices; k++) {
				printf(" %d", c->vertices[k]);
			}
			printf("\n");
		}
	}
	sm_level_t *lv = &m->levels[i];
    printf(" Layer %d: Vertexes: %d - %d\n", i, lv->firstVertex, lv->nVertices);
    for (i = 0; i < m->nTris; i++) {
		sm_tri *t = &m->tris[i];
		printf(" Tri %d: %d %d %d\n", i, m->indices[t->indicesStart], m->indices[t->indicesStart + 1], m->indices[t->indicesStart + 2]);
	}
};
#endif

const int FAN_SIZE = 5;

static void loadFanChunk(sm_model_t *model, sm_chunk_t *chunk, sm_level_t *apexLevel, sm_level_t *nextLevel, bool backwards) {
    chunk->type         = GL_TRIANGLE_FAN;
    chunk->nVertices    = FAN_SIZE + 2;
    chunk->vertices     = (int*)calloc(chunk->nVertices, sizeof(int));
    chunk->backwards    = backwards;
	DBGPRINT("  Creating fan of %d vertices from levels starting %d %d\n", chunk->nVertices, apexLevel->firstVertex, nextLevel->firstVertex);

    chunk->vertices[0]  = apexLevel->firstVertex;
    int i;
    for (i = 0; i < FAN_SIZE; i++) 
		chunk->vertices[i + 1]  = nextLevel->firstVertex + i;
	chunk->vertices[i + 1] = nextLevel->firstVertex;
    
    for (i = 1; i < FAN_SIZE; i++)
		model->addTri(chunk->vertices[0], chunk->vertices[i], chunk->vertices[i + 1], backwards);
	model->addTri(chunk->vertices[0], chunk->vertices[i], chunk->vertices[i + 1], backwards);
}

static void loadStripChunk(sm_model_t *model, sm_chunk_t *chunk, sm_level_t *l0, sm_level_t *l1, int s0, int s1, int nVertices, bool backwards) {
    chunk->type         = GL_TRIANGLE_STRIP;
    chunk->nVertices    = nVertices;
    chunk->vertices     = (int*)calloc(chunk->nVertices, sizeof(int));
    chunk->backwards    = backwards;

	DBGPRINT("  Creating strip of %d vertices from levels starting %d %d\n", nVertices, s0, s1);
	for (int i = 0; i < nVertices; i++) {
		if (i%2 == 0) {
			chunk->vertices[i] = s0; 
			s0 = l0->incIdx(s0, 1);
		}
		else {
			chunk->vertices[i] = s1; 
			s1 = l1->incIdx(s1, 1);
		}

		DBGPRINT("   adding vertex %d\n", chunk->vertices[i]);
	}
	
	for (int i = 2; i < nVertices; i++) {
		model->addTri(chunk->vertices[i - 2], chunk->vertices[i - 1], chunk->vertices[i], backwards);
		backwards = !backwards;
	}
}

const float phi = 1.618033989;	 /* (1 + sqrt(5)) / 2) */
const float r   = 1.902113033;   /* sqrt(1^2 + phi^2)  */

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


sm_model_t *sm_getUnitIsocahedron() {
	//DBGPRINT("Calculating isocahedron\n");
    sm_model_t *ret = createModel(sizeof(icosahedron)/sizeof(M3DVector3f), 3, 3, 20);
    memcpy(ret->vertices, icosahedron, sizeof(icosahedron));

    ret->levels[0].firstVertex = 0;
    ret->levels[0].nVertices   = 1;
    ret->layers[0].firstChunk  = 0;
    ret->layers[0].nChunks     = 1;
    
    ret->levels[1].firstVertex = ret->levels[0].followingVertex();
    ret->levels[1].nVertices   = 5;
    ret->layers[1].firstChunk  = ret->layers[0].followingChunk();
    ret->layers[1].nChunks     = 1;

    ret->levels[2].firstVertex = ret->levels[1].followingVertex();
    ret->levels[2].nVertices   = 5;
    ret->layers[2].firstChunk  = ret->layers[1].followingChunk();
    ret->layers[2].nChunks     = 1;

    ret->levels[3].firstVertex = ret->levels[2].followingVertex();
    ret->levels[3].nVertices   = 1;

	loadFanChunk(ret,   &ret->chunks[0], &ret->levels[0], &ret->levels[1], false); 
	loadStripChunk(ret, &ret->chunks[1], &ret->levels[1], &ret->levels[2], 1, 8, 12, false); 
	loadFanChunk(ret,   &ret->chunks[2], &ret->levels[3], &ret->levels[2], true); 

	DBGPRINTMODEL(ret);
    return ret;
}

void sm_renderIcosahedronFrame() {
	sm_model_t *m = sm_getUnitIsocahedron();
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
static inline int getNewNodeFromOld(sm_level_t *oldL, sm_level_t *newL, int oldNode) {
	int off = oldNode - oldL->firstVertex;
	return newL->firstVertex + 2*off;
}

// Create a new, larger model suitable for containing the more accurate version of the given old model
// Allocate the levels, layers and chunks at the same time, and assign vertices and chunks to levels and layers respectively.
// New vertices are interpolated and assigned to chunks elsewhere.
static sm_model_t *createNewModelFromOld(sm_model_t *oldModel) {
	int i;

	int nChunks = 0;

	for (i = 0; i < oldModel->nChunks; i++) {
		if (oldModel->chunks[i].type == GL_TRIANGLE_FAN)        // Each fan becomes a fan + n strips where n is the number of tris;
			nChunks += 1 + oldModel->chunks[i].nVertices - 2;
		else if (oldModel->chunks[i].type == GL_TRIANGLE_STRIP)	// Each strip becomes 2 strips.
			nChunks += 2;
	}

	// Every edge in the old model aquires a new vertex.  
	// There are 3 edges per tri, but each edge is shared  by 2 tris.
	sm_model_t *newModel = createModel(oldModel->nVertices + oldModel->nTris * 3 / 2, oldModel->nLayers*2, nChunks, oldModel->nTris*4);

	DBGPRINT("Create new from old (%d vertices, %d layers, %d chunks, %d tris) -> new (%d vertices, %d layers, %d chunks, %d tris)\n", 
			oldModel->nVertices, oldModel->nLayers, oldModel->nChunks, oldModel->nTris, 
			newModel->nVertices, newModel->nLayers, newModel->nChunks, newModel->nTris);

	newModel->levels[0].firstVertex = 0;
	newModel->levels[0].nVertices   = 1;

	newModel->layers[0].firstChunk = 0;
	newModel->layers[0].nChunks    = 1;

	newModel->levels[1].firstVertex = newModel->levels[0].followingVertex();
	newModel->levels[1].nVertices   = oldModel->chunks[0].nVertices - 2;

	newModel->layers[1].firstChunk = newModel->layers[0].followingChunk();
	newModel->layers[1].nChunks    = oldModel->chunks[0].nVertices - 2;

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
	newModel->layers[2*i].nChunks    = oldModel->chunks[oldModel->nChunks - 1].nVertices - 2;

	newModel->levels[2*i + 1].firstVertex = newModel->levels[2*i].followingVertex();
	newModel->levels[2*i + 1].nVertices   = oldModel->chunks[oldModel->nChunks - 1].nVertices - 2;
	
	newModel->layers[2*i + 1].firstChunk = newModel->layers[2*i].followingChunk();
	newModel->layers[2*i + 1].nChunks    = 1;
	
	newModel->levels[2*i + 2].firstVertex = newModel->levels[2*i + 1].followingVertex();
	newModel->levels[2*i + 2].nVertices   = 1;

	return newModel;
}

// Create a new, more accurate level by creating vertices bisecting each vertex in the old level.
static void subdivideLevel(sm_model_t *newM, int newLIdx, sm_model_t *oldM, int oldLIdx) {
	sm_level_t *newL = &newM->levels[newLIdx];
	sm_level_t *oldL = &oldM->levels[oldLIdx];

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
static void createNewLevel(sm_model_t *newM, int newLIdx, sm_model_t *oldM, int oldLIdx) {
	sm_level_t *newL = &newM->levels[newLIdx];
	sm_layer_t *oldL = &oldM->layers[oldLIdx];

	DBGPRINT(" Processing chunks in layer %d to get level %d (%d %d)\n", 
	         oldLIdx, newLIdx, newL->firstVertex, newL->nVertices);
	int newV = newL->firstVertex;
	for (int i = 0; i < oldL->nChunks; i++) {
		DBGPRINT("  Processing chunk %d\n", oldL->firstChunk + i);
		sm_chunk_t *c = &oldM->chunks[oldL->firstChunk + i];
		
		for (int j = 0; j < c->nVertices - 2; j++, newV++) {
			calculateBisector(newM->vertices[newV], oldM->vertices[c->vertices[j]], oldM->vertices[c->vertices[j + 1]]); 
		}
	}
}

static void recreateFan(sm_model_t *newModel, int newLApex, int newLNext, int newLLast, 
						sm_model_t *oldModel, int oldLApex, int oldLNext) {
	DBGPRINT(" Converting fan from layers (%d %d) to get layers (%d %d %d)\n", 
	         oldLApex, oldLNext, newLApex, newLNext, newLLast);
	int newApex = newModel->levels[newLApex].firstVertex;
	int oldApex = oldModel->levels[oldLApex].firstVertex;
	m3dCopyVector3(newModel->vertices[newApex], oldModel->vertices[oldApex]);

	sm_chunk_t *oldFan =  &oldModel->chunks[oldModel->layers[(oldLApex < oldLNext) ? oldLApex : oldLNext].firstChunk];  
	sm_chunk_t *newFan =  &newModel->chunks[newModel->layers[(newLApex < newLNext) ? newLApex : newLNext].firstChunk];  
	loadFanChunk(newModel, newFan, &newModel->levels[newLApex], &newModel->levels[newLNext], oldFan->backwards);
	
	int i, oldV, newV, newV2;
	int newChunkIdx = newModel->layers[(newLNext < newLLast) ? newLNext : newLLast].firstChunk;

	for (i = 0, oldV = oldModel->levels[oldLNext].firstVertex, 
		 newV  = newModel->levels[newLNext].firstVertex,
		 newV2 = newModel->levels[newLLast].firstVertex; 
		 i < oldModel->levels[oldLNext].nVertices;
		 i++, oldV++, newV++, newV2+=2, newChunkIdx++) {
		calculateBisector(newModel->vertices[newV], oldModel->vertices[oldApex], oldModel->vertices[oldV]);
		loadStripChunk(newModel, &newModel->chunks[newChunkIdx], &newModel->levels[newLLast], &newModel->levels[newLNext],
					   newV2, newV, FAN_SIZE, !oldFan->backwards);
	}
}

static void recreateLayer(sm_model_t *newModel, int newLayer0, int newLayer1, int newL0, int newL1, int newL2,  
						  sm_model_t *oldModel, int oldLayer, int oldL0, int oldL1) {
	DBGPRINT(" Processing layer %d (%d %d) to get layers %d %d (%d %d %d)\n", 
	         oldLayer, oldL0, oldL1, newLayer0, newLayer1, newL0, newL1, newL2);
	
	int nv1 = newModel->levels[newL1].firstVertex;

	for (int i = 0; i < oldModel->layers[oldLayer].nChunks; i++) {
		sm_chunk_t *oldChunk  = oldModel->getChunk(oldLayer,  i);
		sm_chunk_t *newChunk0 = newModel->getChunk(newLayer0, i);
		sm_chunk_t *newChunk1 = newModel->getChunk(newLayer1, i);
		int ov0 = oldChunk->vertices[0];
		int ov1 = oldChunk->vertices[1];
		int nv0 = getNewNodeFromOld(&oldModel->levels[oldL0], &newModel->levels[newL0], ov0); 
		int nv2 = getNewNodeFromOld(&oldModel->levels[oldL1], &newModel->levels[newL2], ov1);

		int nVertices = (oldChunk->nVertices - 2) * 2 + 2; 

		if (oldChunk->nVertices % 2 == 0) {
			loadStripChunk(newModel, newChunk0, &newModel->levels[newL0], &newModel->levels[newL1],
						   nv0, nv1, nVertices, oldChunk->backwards);
			loadStripChunk(newModel, newChunk1, &newModel->levels[newL1], &newModel->levels[newL2],
						   nv1, nv2, nVertices, oldChunk->backwards);
		}
		else {
			loadStripChunk(newModel, newChunk0, &newModel->levels[newL0], &newModel->levels[newL1],
						   nv0, nv1, nVertices + 1, oldChunk->backwards);
			loadStripChunk(newModel, newChunk1, &newModel->levels[newL1], &newModel->levels[newL2],
						   nv1, nv2, nVertices - 1, oldChunk->backwards);
		}
		
		nv1 = newModel->levels[newL1].incIdx(nv1, oldChunk->nVertices - 2);
	}
}

sm_model_t *sm_getUnitSphere(int precision) {
	if (precision == 0)
		return sm_getUnitIsocahedron();

	sm_model_t *oldModel = sm_getUnitSphere(precision - 1);

	DBGPRINT("Calculating sphere precision %d\n", precision);

	sm_model_t *newModel = createNewModelFromOld(oldModel);
	int i;

	recreateFan(newModel, 0, 1, 2, oldModel, 0, 1);
	for (i = 1; i < oldModel->nLayers - 1; i++) {
		subdivideLevel(newModel, 2*i, oldModel, i);
		createNewLevel(newModel, 2*i+1, oldModel, i);

		sm_layer_t *oldLayer = &oldModel->layers[i];
		sm_chunk_t *oldChunk = &oldModel->chunks[oldLayer->firstChunk];
		if (oldChunk->vertices[0] < oldChunk->vertices[1])
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
