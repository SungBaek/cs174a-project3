#pragma once
#include "Utilities.h"

struct ShapeData	{	GLuint vao;
						int numVertices;	};

void generateCube(GLuint program, ShapeData* cubeData);
void generateSphere(GLuint program, ShapeData* sphereData);
void generateCone(GLuint program, ShapeData* coneData);
void generateCylinder(GLuint program, ShapeData* cylData);
void generateLeaf(GLuint program, ShapeData* leafData);

//----------------------------------------------------------------------------
// Cube

const int numCubeVertices = 36; //(6 faces)(2 triangles/face)(3 vertices/triangle)

point4 cubePoints [numCubeVertices];
point3 cubeNormals[numCubeVertices];
point2 cubeUV     [numCubeVertices];

// Vertices of a unit cube centered at origin, sides aligned with axes
point4 vertices[8] = {
    point4( -0.5, -0.5,  0.5, 1 ),
    point4( -0.5,  0.5,  0.5, 1 ),
    point4(  0.5,  0.5,  0.5, 1 ),
    point4(  0.5, -0.5,  0.5, 1 ),
    point4( -0.5, -0.5, -0.5, 1 ),
    point4( -0.5,  0.5, -0.5, 1 ),
    point4(  0.5,  0.5, -0.5, 1 ),
    point4(  0.5, -0.5, -0.5, 1 )
};

// quad generates two triangles for each face and assigns normals and texture coordinates
//    to the vertices
int Index = 0;
void quad( int a, int b, int c, int d, const point3& normal )
{
    cubePoints[Index] = vertices[a]; cubeNormals[Index] = normal; 
    cubeUV[Index] = point2(0, 1); Index++;
    cubePoints[Index] = vertices[b]; cubeNormals[Index] = normal;
    cubeUV[Index] = point2(0, 0); Index++;
    cubePoints[Index] = vertices[c]; cubeNormals[Index] = normal;
    cubeUV[Index] = point2(1, 0); Index++;
    cubePoints[Index] = vertices[a]; cubeNormals[Index] = normal;
    cubeUV[Index] = point2(0, 1); Index++;
    cubePoints[Index] = vertices[c]; cubeNormals[Index] = normal;
    cubeUV[Index] = point2(1, 0); Index++;
    cubePoints[Index] = vertices[d]; cubeNormals[Index] = normal;
    cubeUV[Index] = point2(1, 1); Index++;
}

// generate 12 triangles: 36 vertices, 36 normals, 36 texture coordinates
void colorcube()
{
    quad( 1, 0, 3, 2, point3( 0,  0,  1) );
    quad( 2, 3, 7, 6, point3( 1,  0,  0) );
    quad( 3, 0, 4, 7, point3( 0, -1,  0) );
    quad( 6, 5, 1, 2, point3( 0,  1,  0) );
    quad( 4, 5, 6, 7, point3( 0,  0, -1) );
    quad( 5, 4, 0, 1, point3(-1,  0,  0) );
}

// initialization
void generateCube(GLuint program, ShapeData* cubeData)
{
    colorcube();
    cubeData->numVertices = numCubeVertices;

    // Create a vertex array object
    glGenVertexArrays( 1, &cubeData->vao );
    glBindVertexArray( cubeData->vao );

    // Set vertex attributes
    setVertexAttrib(program, 
        (float*)cubePoints,  sizeof(cubePoints), 
        (float*)cubeNormals, sizeof(cubeNormals),
        (float*)cubeUV,      sizeof(cubeUV));
}
//----------------------------------------------------------------------------
//leaf
const int numLeafVertices = 30*3*2; //30 triangles * 3 vertices per triangle * 2
const int leafOutlineSize = 32;

point4 leafPoints[numLeafVertices];
point3 leafNormals[numLeafVertices];

int leafIndex = 0;
point4 leafOutline[leafOutlineSize] = {

	point4(0.8f, 0.07f, 0.0f, 1.0f),
	point4(0.8f, 0.08f, 0. - 0.01f, 1.0f),
	point4(0.7f, 0.06f, 0.04f, 1.0f),
	point4(0.7f, 0.07f, -0.03f, 1.0f),

	point4(0.6f, 0.07f, 0.07f, 1.0f),
	point4(0.6f, 0.08f, -.08f, 1.0f),
	point4(0.5f, 0.13f, 0.12f, 1.0f),
	point4(0.5f, 0.14f, -0.15f, 1.0f),

	point4(0.4f, 0.18f, 0.18f, 1.0f),
	point4(0.4f, 0.19f, -0.2f, 1.0f),
	point4(0.3f, 0.21f, .25f, 1.0f),
	point4(0.3f, 0.22f, -.24f, 1.0f),

	point4(0.2f, 0.22f, 0.3f, 1.0f),
	point4(0.2f, 0.23f, -.26f, 1.0f),
	point4(0.1f, 0.21f, .32f, 1.0f),
	point4(0.1f, 0.22f, -.28f, 1.0f),

	point4(0.0f, 0.18f, 0.325, 1.0f),
	point4(0.0f, 0.18f, -.3f, 1.0f),
	point4(-0.1f, 0.16f, 0.32f, 1.0f),
	point4(-0.1f, 0.17f, -.29f, 1.0f),

	point4(-0.2f, 0.14f, 0.3f, 1.0f),
	point4(-0.2f, 0.15f, -.28f, 1.0f),
	point4(-0.3f, 0.13f, 0.25f, 1.0f),
	point4(-0.3f, 0.14f, -.21f, 1.0f),

	point4(-0.4f, 0.12f, 0.18f, 1.0f),
	point4(-0.4f, 0.13f, -.14f, 1.0f),
	point4(-0.5f, 0.11f, 0.0f, 1.0f),
	point4(-0.5f, 0.12f, -0.03f, 1.0f),

	point4(-0.5f, 0.11f, 0.0f, 1.0f),
	point4(-0.5f, 0.11f, -0.03f, 1.0f),
	point4(-0.8f, 0.11f, 0.0f, 1.0f),
	point4(-0.8f, 0.11f, -0.03f, 1.0f)
	//stem end
};

point3 crossProduct(point4 a, point4 b, point4 c)
{
	// | i    j    k |
	// | a.x a.y a.z |
	// | b.x b.y b.z |
	point4 lineA(b.x - a.x, b.y - a.y, b.z - a.z, 0);
	point4 lineB(c.x - a.x, c.y - a.y, c.z - a.z, 0);

	return point3(lineA.y * lineB.z - lineB.y*lineA.z,
		-(lineA.x*lineB.z - lineA.z*lineB.x),
		lineA.x*lineB.y - lineA.y*lineB.x);
}

//put in four corners and get 2 triangles "like quad"
void quadLeaf(point4 a, point4 b, point4 c, point4 d)
{
	point3 normal = crossProduct(a, b, c);

	leafPoints[leafIndex] = a;
	leafNormals[leafIndex] = normal; leafIndex++;
	leafPoints[leafIndex] = b;
	leafNormals[leafIndex] = normal; leafIndex++;
	leafPoints[leafIndex] = c;
	leafNormals[leafIndex] = normal; leafIndex++;

	normal = crossProduct(a, c, d);
	leafPoints[leafIndex] = a;
	leafNormals[leafIndex] = normal; leafIndex++;
	leafPoints[leafIndex] = c;
	leafNormals[leafIndex] = normal; leafIndex++;
	leafPoints[leafIndex] = d;
	leafNormals[leafIndex] = normal; leafIndex++;
}


void generateLeaf(GLuint program, ShapeData* leafData)
{
	int counter = 0;
	while (counter < leafOutlineSize - 2)
	{
		quadLeaf(leafOutline[counter], leafOutline[counter + 1],
			leafOutline[counter + 3], leafOutline[counter + 2]);
		counter = counter + 2; //increment by two each time
	}
	counter = 0;
	//other side of the leaf
	
	while (counter < leafOutlineSize - 2)
	{
		quadLeaf(leafOutline[counter], leafOutline[counter + 1],
			leafOutline[counter + 3], leafOutline[counter + 2]);
		counter = counter + 2; //increment by two each time
	}
	for (int i = numLeafVertices / 2; i < numLeafVertices; i++)
	{
		leafPoints[i].y = leafPoints[i].y - .01f;
		leafNormals[i] = -leafNormals[i];
	}

	leafData->numVertices = numLeafVertices;

	// Create a vertex array object
	glGenVertexArrays(1, &leafData->vao);
	glBindVertexArray(leafData->vao);

	// Set vertex attributes
	setVertexAttrib(program,
		(float*)leafPoints, sizeof(leafPoints),
		(float*)leafNormals, sizeof(leafNormals),
		0, 0);
}


//----------------------------------------------------------------------------
// Sphere approximation by recursive subdivision of a tetrahedron

const int N = 5;  // number of subdivisions
const int numSphereVertices = 16*256*3;  // number of resulting points

point4 spherePoints [numSphereVertices];
point3 sphereNormals[numSphereVertices];
point2 sphereUVs[numSphereVertices];

// four equally spaced points on the unit circle

point4 v[4] = {
    vec4( 0, 0, 1, 1 ), 
    vec4(0.0, 0.942809, -0.333333, 1.0),
    vec4(-0.816497, -0.471405, -0.333333, 1.0),
    vec4(0.816497, -0.471405, -0.333333, 1.0)
};

static int k = 0;

// move a point to unit circle

point4 unit(const point4 &p)
{
    point4 c;
    double d=0;
    for(int i=0; i<3; i++) d+=p[i]*p[i];
    d=sqrt(d);
    if(d > 0) for(int i=0; i<3; i++) c[i] = p[i]/d;
    c[3] = 1;
    return c;
}

void triangle(point4 a, point4 b, point4 c)
{
    spherePoints[k] = a;
    k++;
    spherePoints[k] = b;
    k++;
    spherePoints[k] = c;
    k++;
}

void divide_triangle(point4 a, point4 b, point4 c, int n)
{
    point4 v1, v2, v3;
    if(n>0)
    {
        v1 = unit(a + b);
        v2 = unit(a + c);
        v3 = unit(b + c);   
        divide_triangle(a , v2, v1, n-1);
        divide_triangle(c , v3, v2, n-1);
        divide_triangle(b , v1, v3, n-1);
        divide_triangle(v1, v2, v3, n-1);
    }
    else triangle(a, b, c);
}

void tetrahedron(int n)
{
    divide_triangle(v[0], v[1], v[2], n);
    divide_triangle(v[3], v[2], v[1], n);
    divide_triangle(v[0], v[3], v[1], n);
    divide_triangle(v[0], v[3], v[2], n);
}

// initialization

void generateSphere(GLuint program, ShapeData* sphereData)
{
    tetrahedron(N);

    sphereData->numVertices = numSphereVertices;

    // Normals
    for (int i = 0; i < numSphereVertices; i++)
    {
        sphereNormals[i] = point3(spherePoints[i].x, spherePoints[i].y, spherePoints[i].z);
    }
    
    // TexCoords
    double u, v;
    for (int i = 0; i < numSphereVertices; i++)
    {
        v = 0.5 - asin(spherePoints[i].y)/PI ; //0~1
        u = 0.5*(atan2(spherePoints[i].z,spherePoints[i].x)/PI + 1); //0~1
        sphereUVs[i] = point2(u,v);
    }

    // Create a vertex array object
    glGenVertexArrays( 1, &sphereData->vao );
    glBindVertexArray( sphereData->vao );

    // Set vertex attributes
    setVertexAttrib(program,
        (float*)spherePoints,  sizeof(spherePoints),
        (float*)sphereNormals, sizeof(sphereNormals),
        (float*)sphereUVs, sizeof(sphereUVs));
}
void generateInsideSphere(GLuint program, ShapeData* sphereData)
{
	k = 0;
	tetrahedron(N);

	sphereData->numVertices = numSphereVertices;

	// Normals
	for (int i = 0; i < numSphereVertices; i++)
	{
		sphereNormals[i] = -point3(spherePoints[i].x, spherePoints[i].y, spherePoints[i].z);
	}

	// TexCoords
	double u, v;
	for (int i = 0; i < numSphereVertices; i++)
	{
		v = 0.5 - asin(spherePoints[i].y) / PI; //0~1
		u = 0.5*(atan2(spherePoints[i].z, spherePoints[i].x) / PI + 1); //0~1
		sphereUVs[i] = point2(u, v);
	}

	// Create a vertex array object
	glGenVertexArrays(1, &sphereData->vao);
	glBindVertexArray(sphereData->vao);

	// Set vertex attributes
	setVertexAttrib(program,
		(float*)spherePoints, sizeof(spherePoints),
		(float*)sphereNormals, sizeof(sphereNormals),
		(float*)sphereUVs, sizeof(sphereUVs));
}

//----------------------------------------------------------------------------
// Cone

const int numConeDivisions = 32;
const int numConeVertices = numConeDivisions * 6;

point4 conePoints [numConeVertices];
point3 coneNormals[numConeVertices];

point2 circlePoints[numConeDivisions];

void makeCircle(point2* dest, int numDivisions)
{
    for (int i = 0; i < numDivisions; i++)
    {
        float a = i * 2 * PI / numDivisions;
        dest[i] = point2(cosf(a), sinf(a));
    }
}

void makeConeWall(point4* destp, point3* destn, int numDivisions, float z1, float z2, int& Index, int dir)
{
    for (int i = 0; i < numDivisions; i++)
    {
        point3 p1(circlePoints[i].x, circlePoints[i].y, z1);
        point3 p2(0, 0, z2);
        point3 p3(circlePoints[(i+1)%numDivisions].x, circlePoints[(i+1)%numDivisions].y, z1);
        if (dir == -1)
        {
            point3 temp = p1;
            p1 = p3;
            p3 = temp;
        }
        point3 n = cross(p1-p2, p3-p2);
        destp[Index] = p1; destn[Index] = n; Index++;
        destp[Index] = p2; destn[Index] = n; Index++;
        destp[Index] = p3; destn[Index] = n; Index++;
    }    
}

void generateCone(GLuint program, ShapeData* coneData)
{
    makeCircle(circlePoints, numConeDivisions);
    int Index = 0;
    makeConeWall(conePoints, coneNormals, numConeDivisions, 1, 1, Index, 1);
    makeConeWall(conePoints, coneNormals, numConeDivisions, 1, -1, Index, -1);
    
    coneData->numVertices = numConeVertices;

    // Create a vertex array object
    glGenVertexArrays( 1, &coneData->vao );
    glBindVertexArray( coneData->vao );
    
    // Set vertex attributes
    setVertexAttrib(program,
                    (float*)conePoints,  sizeof(conePoints),
                    (float*)coneNormals, sizeof(coneNormals),
                    0, 0 );
}

//----------------------------------------------------------------------------
// Cylinder

const int numCylDivisions = 32;
const int numCylVertices = numCylDivisions * 12;

point4 cylPoints [numCylVertices];
point3 cylNormals[numCylVertices];

void generateCylinder(GLuint program, ShapeData* cylData)
{
    makeCircle(circlePoints, numCylDivisions);
    int Index = 0;
    makeConeWall(cylPoints, cylNormals, numCylDivisions, 1, 1, Index, 1);
    makeConeWall(cylPoints, cylNormals, numCylDivisions, -1, -1, Index, -1);
    
    for (int i = 0; i < numCylDivisions; i++)
    {
        int i2 = (i+1)%numCylDivisions;
        point3 p1(circlePoints[i2].x, circlePoints[i2].y, -1);
        point3 p2(circlePoints[i2].x, circlePoints[i2].y, 1);
        point3 p3(circlePoints[i].x,  circlePoints[i].y,  1);
        //point3 n = cross(p3-p2, p1-p2);
        cylPoints[Index] = p1; cylNormals[Index] = point3(p1.x, p1.y, 0); Index++;
        cylPoints[Index] = p2; cylNormals[Index] = point3(p2.x, p2.y, 0); Index++;
        cylPoints[Index] = p3; cylNormals[Index] = point3(p3.x, p3.y, 0); Index++;
        p1 = point3(circlePoints[i2].x, circlePoints[i2].y, -1);
        p2 = point3(circlePoints[i].x,  circlePoints[i].y,  1);
        p3 = point3(circlePoints[i].x,  circlePoints[i].y,  -1);
        //n = cross(p3-p2, p1-p2);
        cylPoints[Index] = p1; cylNormals[Index] = point3(p1.x, p1.y, 0); Index++;
        cylPoints[Index] = p2; cylNormals[Index] = point3(p2.x, p2.y, 0); Index++;
        cylPoints[Index] = p3; cylNormals[Index] = point3(p3.x, p3.y, 0); Index++;
    }
    
    cylData->numVertices = numCylVertices;
    
    // Create a vertex array object
    glGenVertexArrays( 1, &cylData->vao );
    glBindVertexArray( cylData->vao );
    
    // Set vertex attributes
    setVertexAttrib(program,
                    (float*)cylPoints,  sizeof(cylPoints),
                    (float*)cylNormals, sizeof(cylNormals),
                    0, 0 );
}