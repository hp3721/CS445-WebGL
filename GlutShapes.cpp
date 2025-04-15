#include "SpinningUAH.h"
#include "GlutShapes.h"

#define DECLARE_SHAPE_CACHE_DECOMPOSE_TO_TRIANGLE(name,nameICaps,nameCaps)\
static GLboolean name##Cached = GL_FALSE;\
static GLfloat   name##_verts[nameCaps##_VERT_ELEM_PER_OBJ];\
static GLfloat   name##_norms[nameCaps##_VERT_ELEM_PER_OBJ];\
static GLushort  name##_vertIdxs[nameCaps##_VERT_PER_OBJ_TRI];\
static void fgh##nameICaps##Generate(void)\
{\
fghGenerateGeometryWithIndexArray(nameCaps##_NUM_FACES, nameCaps##_NUM_EDGE_PER_FACE,\
name##_v, name##_vi, name##_n,\
name##_verts, name##_norms, name##_vertIdxs);\
}

/* Version for OpenGL (ES) 1.1 */
static void fghDrawGeometryWire11(GLfloat *vertices, GLfloat *normals,
                                  GLushort *vertIdxs, GLsizei numParts, GLsizei numVertPerPart, GLenum vertexMode,
                                  GLushort *vertIdxs2, GLsizei numParts2, GLsizei numVertPerPart2
    )
{
    int i;

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);

    glVertexPointer(3, GL_FLOAT, 0, vertices);
    glNormalPointer(GL_FLOAT, 0, normals);

    if (!vertIdxs)
        /* Draw per face */
            for (i=0; i<numParts; i++)
                glDrawArrays(vertexMode, i*numVertPerPart, numVertPerPart);
    else
        for (i=0; i<numParts; i++)
            glDrawElements(vertexMode,numVertPerPart,GL_UNSIGNED_SHORT,vertIdxs+i*numVertPerPart);

    if (vertIdxs2)
        for (i=0; i<numParts2; i++)
            glDrawElements(GL_LINE_LOOP,numVertPerPart2,GL_UNSIGNED_SHORT,vertIdxs2+i*numVertPerPart2);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
}

/**
 * Draw geometric shape in wire mode (only edges)
 *
 * Arguments:
 * GLfloat *vertices, GLfloat *normals, GLsizei numVertices
 *   The vertex coordinate and normal buffers, and the number of entries in
 *   those
 * GLushort *vertIdxs
 *   a vertex indices buffer, optional (never passed for the polyhedra)
 * GLsizei numParts, GLsizei numVertPerPart
 *   polyhedra: number of faces, and the number of vertices for drawing
 *     each face
 *   non-polyhedra: number of edges to draw for first subdivision (not
 *     necessarily equal to number of subdivisions requested by user, e.g.
 *     as each subdivision is enclosed by two edges), and number of
 *     vertices for drawing each
 *   numParts * numVertPerPart gives the number of entries in the vertex
 *     array vertIdxs
 * GLenum vertexMode
 *   vertex drawing mode (e.g. always GL_LINE_LOOP for polyhedra, varies
 *   for others)
 * GLushort *vertIdxs2, GLsizei numParts2, GLsizei numVertPerPart2
 *   non-polyhedra only: same as the above, but now for subdivisions along
 *   the other axis. Always drawn as GL_LINE_LOOP.
 *
 * Feel free to contribute better naming ;)
 */
void fghDrawGeometryWire(GLfloat *vertices, GLfloat *normals, GLsizei numVertices,
                                GLushort *vertIdxs, GLsizei numParts, GLsizei numVertPerPart, GLenum vertexMode,
                                GLushort *vertIdxs2, GLsizei numParts2, GLsizei numVertPerPart2
    )
{
    fghDrawGeometryWire11(vertices, normals,
                          vertIdxs, numParts, numVertPerPart, vertexMode,
                          vertIdxs2, numParts2, numVertPerPart2);
}

static void fghDrawGeometrySolid11(GLfloat *vertices, GLfloat *normals, GLfloat *textcs, GLsizei numVertices,
                                   GLushort *vertIdxs, GLsizei numParts, GLsizei numVertIdxsPerPart)
{
    int i;

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);

    glVertexPointer(3, GL_FLOAT, 0, vertices);
    glNormalPointer(GL_FLOAT, 0, normals);

    if (textcs)
    {
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glTexCoordPointer(2, GL_FLOAT, 0, textcs);
    }

    if (!vertIdxs)
        glDrawArrays(GL_TRIANGLES, 0, numVertices);
    else
        if (numParts>1)
            for (i=0; i<numParts; i++)
                glDrawElements(GL_TRIANGLE_STRIP, numVertIdxsPerPart, GL_UNSIGNED_SHORT, vertIdxs+i*numVertIdxsPerPart);
        else
            glDrawElements(GL_TRIANGLES, numVertIdxsPerPart, GL_UNSIGNED_SHORT, vertIdxs);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    if (textcs)
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

/* Draw the geometric shape with filled triangles
 *
 * Arguments:
 * GLfloat *vertices, GLfloat *normals, GLfloat *textcs, GLsizei numVertices
 *   The vertex coordinate, normal and texture coordinate buffers, and the
 *   number of entries in those
 * GLushort *vertIdxs
 *   a vertex indices buffer, optional (not passed for the polyhedra with
 *   triangular faces)
 * GLsizei numParts, GLsizei numVertPerPart
 *   polyhedra: not used for polyhedra with triangular faces
       (numEdgePerFace==3), as each vertex+normal pair is drawn only once,
       so no vertex indices are used.
       Else, the shape was triangulated (DECOMPOSE_TO_TRIANGLE), leading to
       reuse of some vertex+normal pairs, and thus the need to draw with
       glDrawElements. numParts is always 1 in this case (we can draw the
       whole object with one call to glDrawElements as the vertex index
       array contains separate triangles), and numVertPerPart indicates
       the number of vertex indices in the vertex array.
 *   non-polyhedra: number of parts (GL_TRIANGLE_STRIPs) to be drawn
       separately (numParts calls to glDrawElements) to create the object.
       numVertPerPart indicates the number of vertex indices to be
       processed at each draw call.
 *   numParts * numVertPerPart gives the number of entries in the vertex
 *     array vertIdxs
 */
void fghDrawGeometrySolid(GLfloat *vertices, GLfloat *normals, GLfloat *textcs, GLsizei numVertices,
                          GLushort *vertIdxs, GLsizei numParts, GLsizei numVertIdxsPerPart)
{
    fghDrawGeometrySolid11(vertices, normals, textcs, numVertices,
                           vertIdxs, numParts, numVertIdxsPerPart);
}

/**
 * Generate all combinations of vertices and normals needed to draw object.
 * Optional shape decomposition to triangles:
 * We'll use glDrawElements to draw all shapes that are not naturally
 * composed of triangles, so generate an index vector here, using the
 * below sampling scheme.
 * Be careful to keep winding of all triangles counter-clockwise,
 * assuming that input has correct winding...
 */
static GLubyte   vert4Decomp[6] = {0,1,2, 0,2,3};             /* quad    : 4 input vertices, 6 output (2 triangles) */
static GLubyte   vert5Decomp[9] = {0,1,2, 0,2,4, 4,2,3};      /* pentagon: 5 input vertices, 9 output (3 triangles) */

static void fghGenerateGeometryWithIndexArray(int numFaces, int numEdgePerFace, GLfloat *vertices, GLubyte *vertIndices, GLfloat *normals, GLfloat *vertOut, GLfloat *normOut, GLushort *vertIdxOut)
{
    int i,j,numEdgeIdxPerFace;
    GLubyte   *vertSamps = NULL;
    switch (numEdgePerFace)
    {
    case 3:
        /* nothing to do here, we'll draw with glDrawArrays */
        break;
    case 4:
        vertSamps = vert4Decomp;
        numEdgeIdxPerFace = 6;      /* 6 output vertices for each face */
        break;
    case 5:
        vertSamps = vert5Decomp;
        numEdgeIdxPerFace = 9;      /* 9 output vertices for each face */
        break;
    }
    /*
     * Build array with vertices using vertex coordinates and vertex indices
     * Do same for normals.
     * Need to do this because of different normals at shared vertices.
     */
    for (i=0; i<numFaces; i++)
    {
        int normIdx         = i*3;
        int faceIdxVertIdx  = i*numEdgePerFace; /* index to first element of "row" in vertex indices */
        for (j=0; j<numEdgePerFace; j++)
        {
            int outIdx  = i*numEdgePerFace*3+j*3;
            int vertIdx = vertIndices[faceIdxVertIdx+j]*3;

            vertOut[outIdx  ] = vertices[vertIdx  ];
            vertOut[outIdx+1] = vertices[vertIdx+1];
            vertOut[outIdx+2] = vertices[vertIdx+2];

            normOut[outIdx  ] = normals [normIdx  ];
            normOut[outIdx+1] = normals [normIdx+1];
            normOut[outIdx+2] = normals [normIdx+2];
        }

        /* generate vertex indices for each face */
        if (vertSamps)
            for (j=0; j<numEdgeIdxPerFace; j++)
                vertIdxOut[i*numEdgeIdxPerFace+j] = faceIdxVertIdx + vertSamps[j];
    }
}

/* -- Cube -- */
#define CUBE_NUM_VERT           8
#define CUBE_NUM_FACES          6
#define CUBE_NUM_EDGE_PER_FACE  4
#define CUBE_VERT_PER_OBJ       (CUBE_NUM_FACES*CUBE_NUM_EDGE_PER_FACE)
#define CUBE_VERT_ELEM_PER_OBJ  (CUBE_VERT_PER_OBJ*3)
#define CUBE_VERT_PER_OBJ_TRI   (CUBE_VERT_PER_OBJ+CUBE_NUM_FACES*2)    /* 2 extra edges per face when drawing quads as triangles */
/* Vertex Coordinates */
static GLfloat cube_v[CUBE_NUM_VERT*3] =
{
    .5f, .5f, .5f,
   -.5f, .5f, .5f,
   -.5f,-.5f, .5f,
    .5f,-.5f, .5f,
    .5f,-.5f,-.5f,
    .5f, .5f,-.5f,
   -.5f, .5f,-.5f,
   -.5f,-.5f,-.5f
};
/* Normal Vectors */
static GLfloat cube_n[CUBE_NUM_FACES*3] =
{
    0.0f, 0.0f, 1.0f,
    1.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
   -1.0f, 0.0f, 0.0f,
    0.0f,-1.0f, 0.0f,
    0.0f, 0.0f,-1.0f
};

/* Vertex indices, as quads, before triangulation */
static GLubyte cube_vi[CUBE_VERT_PER_OBJ] =
{
    0,1,2,3,
    0,3,4,5,
    0,5,6,1,
    1,6,7,2,
    7,4,3,2,
    4,7,6,5
};
DECLARE_SHAPE_CACHE_DECOMPOSE_TO_TRIANGLE(cube,Cube,CUBE)

static void fghCube( GLfloat dSize, GLboolean useWireMode )
{
    GLfloat *vertices;

    if (!cubeCached)
    {
        fghCubeGenerate();
        cubeCached = GL_TRUE;
    }

    if (dSize!=1.f)
    {
        /* Need to build new vertex list containing vertices for cube of different size */
        int i;

        vertices = static_cast<GLfloat*>(malloc(CUBE_VERT_ELEM_PER_OBJ * sizeof(GLfloat)));

        /* Bail out if memory allocation fails, fgError never returns */
        if (!vertices)
        {
            free(vertices);
        }

        for (i=0; i<CUBE_VERT_ELEM_PER_OBJ; i++)
            vertices[i] = dSize*cube_verts[i];
    }
    else
        vertices = cube_verts;

    if (useWireMode)
        fghDrawGeometryWire(vertices, cube_norms, CUBE_VERT_PER_OBJ,
                            NULL,CUBE_NUM_FACES, CUBE_NUM_EDGE_PER_FACE,GL_LINE_LOOP,
                            NULL,0,0);
    else
        fghDrawGeometrySolid(vertices, cube_norms, NULL, CUBE_VERT_PER_OBJ,
                             cube_vertIdxs, 1, CUBE_VERT_PER_OBJ_TRI);

    if (dSize!=1.f)
        /* cleanup allocated memory */
            free(vertices);
}

/* -- Now the various non-polyhedra (shapes involving circles) -- */
/*
 * Compute lookup table of cos and sin values forming a circle
 * (or half circle if halfCircle==TRUE)
 *
 * Notes:
 *    It is the responsibility of the caller to free these tables
 *    The size of the table is (n+1) to form a connected loop
 *    The last entry is exactly the same as the first
 *    The sign of n can be flipped to get the reverse loop
 */
static void fghCircleTable(GLfloat **sint, GLfloat **cost, const int n, const GLboolean halfCircle)
{
    int i;

    /* Table size, the sign of n flips the circle direction */
    const int size = abs(n);

    /* Determine the angle between samples */
    const GLfloat angle = (halfCircle?1:2)*(GLfloat)M_PI/(GLfloat)( ( n == 0 ) ? 1 : n );

    /* Allocate memory for n samples, plus duplicate of first entry at the end */
    *sint = static_cast<float*>(malloc(sizeof(GLfloat) * (size+1)));
    *cost = static_cast<float*>(malloc(sizeof(GLfloat) * (size+1)));

    /* Bail out if memory allocation fails, fgError never returns */
    if (!(*sint) || !(*cost))
    {
        free(*sint);
        free(*cost);
    }

    /* Compute cos and sin around the circle */
    (*sint)[0] = 0.0;
    (*cost)[0] = 1.0;

    for (i=1; i<size; i++)
    {
        (*sint)[i] = (GLfloat)sin(angle*i);
        (*cost)[i] = (GLfloat)cos(angle*i);
    }


    if (halfCircle)
    {
        (*sint)[size] =  0.0f;  /* sin PI */
        (*cost)[size] = -1.0f;  /* cos PI */
    }
    else
    {
        /* Last sample is duplicate of the first (sin or cos of 2 PI) */
        (*sint)[size] = (*sint)[0];
        (*cost)[size] = (*cost)[0];
    }
}

static void fghGenerateSphere(GLfloat radius, GLint slices, GLint stacks, GLfloat **vertices, GLfloat **normals, int* nVert)
{
    int i,j;
    int idx = 0;    /* idx into vertex/normal buffer */
    GLfloat x,y,z;

    /* Pre-computed circle */
    GLfloat *sint1,*cost1;
    GLfloat *sint2,*cost2;

    /* number of unique vertices */
    if (slices==0 || stacks<2)
    {
        /* nothing to generate */
        *nVert = 0;
        return;
    }
    *nVert = slices*(stacks-1)+2;

    /* precompute values on unit circle */
    fghCircleTable(&sint1,&cost1,-slices,GL_FALSE);
    fghCircleTable(&sint2,&cost2, stacks,GL_TRUE);

    /* Allocate vertex and normal buffers, bail out if memory allocation fails */
    *vertices = static_cast<float*>(malloc(*nVert*3*sizeof(GLfloat)));
    *normals  = static_cast<float*>(malloc(*nVert*3*sizeof(GLfloat)));
    if (!(*vertices) || !(*normals))
    {
        free(*vertices);
        free(*normals);
    }

    /* top */
    (*vertices)[0] = 0.f;
    (*vertices)[1] = 0.f;
    (*vertices)[2] = radius;
    (*normals )[0] = 0.f;
    (*normals )[1] = 0.f;
    (*normals )[2] = 1.f;
    idx = 3;

    /* each stack */
    for( i=1; i<stacks; i++ )
    {
        for(j=0; j<slices; j++, idx+=3)
        {
            x = cost1[j]*sint2[i];
            y = sint1[j]*sint2[i];
            z = cost2[i];

            (*vertices)[idx  ] = x*radius;
            (*vertices)[idx+1] = y*radius;
            (*vertices)[idx+2] = z*radius;
            (*normals )[idx  ] = x;
            (*normals )[idx+1] = y;
            (*normals )[idx+2] = z;
        }
    }

    /* bottom */
    (*vertices)[idx  ] =  0.f;
    (*vertices)[idx+1] =  0.f;
    (*vertices)[idx+2] = -radius;
    (*normals )[idx  ] =  0.f;
    (*normals )[idx+1] =  0.f;
    (*normals )[idx+2] = -1.f;

    /* Done creating vertices, release sin and cos tables */
    free(sint1);
    free(cost1);
    free(sint2);
    free(cost2);
}

static void fghSphere( GLfloat radius, GLint slices, GLint stacks, GLboolean useWireMode )
{
    int i,j,idx, nVert;
    GLfloat *vertices, *normals;

    /* Generate vertices and normals */
    fghGenerateSphere(radius,slices,stacks,&vertices,&normals,&nVert);

    if (nVert==0)
        /* nothing to draw */
        return;

    if (useWireMode)
    {
        GLushort  *sliceIdx, *stackIdx;
        /* First, generate vertex index arrays for drawing with glDrawElements
         * We have a bunch of line_loops to draw for each stack, and a
         * bunch for each slice.
         */

        sliceIdx = static_cast<GLushort*>(malloc(slices * (stacks + 1) * sizeof(GLushort)));
        stackIdx = static_cast<GLushort*>(malloc(slices * (stacks - 1) * sizeof(GLushort)));
        if (!(stackIdx) || !(sliceIdx))
        {
            free(stackIdx);
            free(sliceIdx);
        }

        /* generate for each stack */
        for (i=0,idx=0; i<stacks-1; i++)
        {
            GLushort offset = 1+i*slices;           /* start at 1 (0 is top vertex), and we advance one stack down as we go along */
            for (j=0; j<slices; j++, idx++)
            {
                stackIdx[idx] = offset+j;
            }
        }

        /* generate for each slice */
        for (i=0,idx=0; i<slices; i++)
        {
            GLushort offset = 1+i;                  /* start at 1 (0 is top vertex), and we advance one slice as we go along */
            sliceIdx[idx++] = 0;                    /* vertex on top */
            for (j=0; j<stacks-1; j++, idx++)
            {
                sliceIdx[idx] = offset+j*slices;
            }
            sliceIdx[idx++] = nVert-1;              /* zero based index, last element in array... */
        }

        /* draw */
        fghDrawGeometryWire(vertices,normals,nVert,
            sliceIdx,slices,stacks+1,GL_LINE_STRIP,
            stackIdx,stacks-1,slices);

        /* cleanup allocated memory */
        free(sliceIdx);
        free(stackIdx);
    }
    else
    {
        /* First, generate vertex index arrays for drawing with glDrawElements
         * All stacks, including top and bottom are covered with a triangle
         * strip.
         */
        GLushort  *stripIdx;
        /* Create index vector */
        GLushort offset;

        /* Allocate buffers for indices, bail out if memory allocation fails */
        stripIdx = static_cast<GLushort*>(malloc((slices + 1) * 2 * (stacks) * sizeof(GLushort)));
        if (!(stripIdx))
        {
            free(stripIdx);
        }

        /* top stack */
        for (j=0, idx=0;  j<slices;  j++, idx+=2)
        {
            stripIdx[idx  ] = j+1;              /* 0 is top vertex, 1 is first for first stack */
            stripIdx[idx+1] = 0;
        }
        stripIdx[idx  ] = 1;                    /* repeat first slice's idx for closing off shape */
        stripIdx[idx+1] = 0;
        idx+=2;

        /* middle stacks: */
        /* Strip indices are relative to first index belonging to strip, NOT relative to first vertex/normal pair in array */
        for (i=0; i<stacks-2; i++, idx+=2)
        {
            offset = 1+i*slices;                    /* triangle_strip indices start at 1 (0 is top vertex), and we advance one stack down as we go along */
            for (j=0; j<slices; j++, idx+=2)
            {
                stripIdx[idx  ] = offset+j+slices;
                stripIdx[idx+1] = offset+j;
            }
            stripIdx[idx  ] = offset+slices;        /* repeat first slice's idx for closing off shape */
            stripIdx[idx+1] = offset;
        }

        /* bottom stack */
        offset = 1+(stacks-2)*slices;               /* triangle_strip indices start at 1 (0 is top vertex), and we advance one stack down as we go along */
        for (j=0; j<slices; j++, idx+=2)
        {
            stripIdx[idx  ] = nVert-1;              /* zero based index, last element in array (bottom vertex)... */
            stripIdx[idx+1] = offset+j;
        }
        stripIdx[idx  ] = nVert-1;                  /* repeat first slice's idx for closing off shape */
        stripIdx[idx+1] = offset;


        /* draw */
        fghDrawGeometrySolid(vertices,normals,NULL,nVert,stripIdx,stacks,(slices+1)*2);

        /* cleanup allocated memory */
        free(stripIdx);
    }

    /* cleanup allocated memory */
    free(vertices);
    free(normals);
}

void glutWireCube( double dSize )
{
    fghCube( (GLfloat)dSize, GL_TRUE );
}

void glutSolidCube( double dSize )
{
    fghCube( (GLfloat)dSize, GL_FALSE );
}

void glutSolidSphere(double radius, GLint slices, GLint stacks)
{
    fghSphere( (GLfloat)radius, slices, stacks, GL_FALSE );
}