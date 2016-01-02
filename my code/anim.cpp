// anim.cpp version 5.0 -- Template code for drawing an articulated figure.  CS 174A.
#include "../CS174a template/Utilities.h"
#include "../CS174a template/Shapes.h"

std::stack<mat4> mvstack;

int g_width = 800, g_height = 800 ;
float zoom = 1 ;

int animate = 0, recording = 0, basis_to_display = -1;
double TIME = 0;

const int MAXLEAFCOUNT = 15;
const float INVERSETIME = 33;
const float SOCCERTIME = 8;
const float GRANDPATIME = 27;
const float PROPOSETIME = 33*1.5 + 3;

const unsigned X = 0, Y = 1, Z = 2;

vec4 eye( 0, 0, 15, 1), ref( 0, 0, 0, 1 ), up( 0, 1, 0, 0 );	// The eye point and look-at point.

mat4	orientation, view_trans;
ShapeData cubeData, sphereData, coneData, cylData, leafData, negSphereData;				// Structs that hold the Vertex Array Object index and number of vertices of each shape.
GLuint	texture_cube, texture_earth, texture_sky;
GLint   uModelView, uProjection, uView,
		uAmbient, uDiffuse, uSpecular, uLightPos, uShininess,
		uTex, uEnableTex;

void init()
{
#ifdef EMSCRIPTEN
    GLuint program = LoadShaders( "vshader.glsl", "fshader.glsl" );								// Load shaders and use the resulting shader program
    TgaImage coolImage ("challenge.tga");    
    TgaImage earthImage("earth.tga");

#else
	GLuint program = LoadShaders( "../my code/vshader.glsl", "../my code/fshader.glsl" );		// Load shaders and use the resulting shader program
    TgaImage coolImage ("../my code/grass.tga");    
    TgaImage earthImage("../my code/sky.tga");
#endif
    glUseProgram(program);

	generateCube(program, &cubeData);		// Generate vertex arrays for geometric shapes
    generateSphere(program, &sphereData);
	generateInsideSphere(program, &negSphereData);
    generateCone(program, &coneData);
    generateCylinder(program, &cylData);
	generateLeaf(program, &leafData);

    uModelView  = glGetUniformLocation( program, "ModelView"  );
    uProjection = glGetUniformLocation( program, "Projection" );
    uView		= glGetUniformLocation( program, "View"       );
    uAmbient	= glGetUniformLocation( program, "AmbientProduct"  );
    uDiffuse	= glGetUniformLocation( program, "DiffuseProduct"  );
    uSpecular	= glGetUniformLocation( program, "SpecularProduct" );
    uLightPos	= glGetUniformLocation( program, "LightPosition"   );
    uShininess	= glGetUniformLocation( program, "Shininess"       );
    uTex		= glGetUniformLocation( program, "Tex"             );
    uEnableTex	= glGetUniformLocation( program, "EnableTex"       );

    glUniform4f( uAmbient,    0.2,  0.2,  0.2, 1 );
    glUniform4f( uDiffuse,    0.6,  0.6,  0.6, 1 );
    glUniform4f( uSpecular,   0.2,  0.2,  0.2, 1 );
    glUniform4f( uLightPos,  15.0, 15.0, 30.0, 0 );
    glUniform1f( uShininess, 100);

    glEnable(GL_DEPTH_TEST);
    
    glGenTextures( 1, &texture_cube );
    glBindTexture( GL_TEXTURE_2D, texture_cube );
    
    glTexImage2D(GL_TEXTURE_2D, 0, 4, coolImage.width, coolImage.height, 0,
                 (coolImage.byteCount == 3) ? GL_BGR : GL_BGRA,
                 GL_UNSIGNED_BYTE, coolImage.data );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    
    
    glGenTextures( 1, &texture_earth );
    glBindTexture( GL_TEXTURE_2D, texture_earth );
    
    glTexImage2D(GL_TEXTURE_2D, 0, 4, earthImage.width, earthImage.height, 0,
                 (earthImage.byteCount == 3) ? GL_BGR : GL_BGRA,
                 GL_UNSIGNED_BYTE, earthImage.data );
    
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    
    glUniform1i( uTex, 0);	// Set texture sampler variable to texture unit 0
	
	glEnable(GL_DEPTH_TEST);
}

struct color{ color( float r, float g, float b) : r(r), g(g), b(b) {} float r, g, b;};
std::stack<color> colors;
void set_color(float r, float g, float b)
{
	colors.push(color(r, g, b));

	float ambient  = 0.2, diffuse  = 0.6, specular = 0.2;
    glUniform4f(uAmbient,  ambient*r,  ambient*g,  ambient*b,  1 );
    glUniform4f(uDiffuse,  diffuse*r,  diffuse*g,  diffuse*b,  1 );
    glUniform4f(uSpecular, specular*r, specular*g, specular*b, 1 );
}

int mouseButton = -1, prevZoomCoord = 0 ;
vec2 anchor;
void myPassiveMotionCallBack(int x, int y) {	anchor = vec2( 2. * x / g_width - 1, -2. * y / g_height + 1 ); }

float getTime(float beginTime, float duration, float reverseTime){
//time control
	float tempTime = 5.f;

	if (TIME > beginTime && TIME < beginTime + duration)
		tempTime = TIME - beginTime;
	else if (TIME > beginTime + duration && TIME < reverseTime)
		tempTime = duration;
	else if (TIME < beginTime)
		tempTime = 0;
	else if (TIME > reverseTime && TIME < reverseTime + duration/2.0f)
		tempTime = duration - 2*(TIME - reverseTime);
	else
		tempTime = 0;
	return tempTime;
}
//end time control

void myMouseCallBack(int button, int state, int x, int y)	// start or end mouse interaction
{
    mouseButton = button;
   
    if( button == GLUT_LEFT_BUTTON && state == GLUT_UP )
        mouseButton = -1 ;
    if( button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN )
        prevZoomCoord = -2. * y / g_height + 1;

    glutPostRedisplay() ;
}

void myMotionCallBack(int x, int y)
{
	vec2 arcball_coords( 2. * x / g_width - 1, -2. * y / g_height + 1 );
	 
    if( mouseButton == GLUT_LEFT_BUTTON )
    {
	   orientation = RotateX( -10 * (arcball_coords.y - anchor.y) ) * orientation;
	   orientation = RotateY(  10 * (arcball_coords.x - anchor.x) ) * orientation;
    }
	
	if( mouseButton == GLUT_RIGHT_BUTTON )
		zoom *= 1 + .1 * (arcball_coords.y - anchor.y);
    glutPostRedisplay() ;
}

void idleCallBack(void)
{
    if( !animate ) return;
	double prev_time = TIME;
    TIME = TM.GetElapsedTime() ;
	if( prev_time == 0 ) TM.Reset();
    glutPostRedisplay() ;
}
void drawLeaf(void)//make a leafff
{
	glUniformMatrix4fv(uModelView, 1, GL_FALSE, transpose(view_trans));
	glBindVertexArray(leafData.vao);
	glDrawArrays(GL_TRIANGLES, 0, leafData.numVertices);
}

void drawCylinder()	//render a solid cylinder oriented along the Z axis; bases are of radius 1, placed at Z = 0, and at Z = 1.
{
    glUniformMatrix4fv( uModelView, 1, GL_FALSE, transpose(view_trans) );
    glBindVertexArray( cylData.vao );
    glDrawArrays( GL_TRIANGLES, 0, cylData.numVertices );
}

void drawCone()	//render a solid cone oriented along the Z axis; bases are of radius 1, placed at Z = 0, and at Z = 1.
{
    glUniformMatrix4fv( uModelView, 1, GL_FALSE, transpose(view_trans) );
    glBindVertexArray( coneData.vao );
    glDrawArrays( GL_TRIANGLES, 0, coneData.numVertices );
}

void drawGroundCube()		// draw a cube with dimensions 1,1,1 centered around the origin.
{
	glBindTexture( GL_TEXTURE_2D, texture_cube );
    glUniform1i( uEnableTex, 1 );
    glUniformMatrix4fv( uModelView, 1, GL_FALSE, transpose(view_trans) );
    glBindVertexArray( cubeData.vao );
    glDrawArrays( GL_TRIANGLES, 0, cubeData.numVertices );
    glUniform1i( uEnableTex, 0 );
}

void drawCube()
{
	glUniformMatrix4fv(uModelView, 1, GL_FALSE, transpose(view_trans));
	glBindVertexArray(cubeData.vao);
	glDrawArrays(GL_TRIANGLES, 0, cubeData.numVertices);
}

void drawSphere()
{
	glUniformMatrix4fv(uModelView, 1, GL_FALSE, transpose(view_trans));
	glBindVertexArray(sphereData.vao);
	glDrawArrays(GL_TRIANGLES, 0, sphereData.numVertices);
}

void drawSkySphere()	// draw a sphere with radius 1 centered around the origin.
{ 
	glBindTexture( GL_TEXTURE_2D, texture_earth);
    glUniform1i( uEnableTex, 1);
    glUniformMatrix4fv( uModelView, 1, GL_FALSE, transpose(view_trans) );
    glBindVertexArray( negSphereData.vao );
    glDrawArrays( GL_TRIANGLES, 0, negSphereData.numVertices );
    glUniform1i( uEnableTex, 0 );
}


void drawTree(int treeN)
{
	mvstack.push(view_trans);
	if (treeN == 0)
	{
		set_color(0.647f, 0.164f, 0.164f);
		mvstack.push(view_trans);
		view_trans *= Scale(3, 8, 3);
		view_trans *= RotateX(90);
		drawCone();
		view_trans = mvstack.top(); mvstack.pop();

		view_trans *= Translate(0, 2, 0);
		view_trans *= RotateX(6);
		mvstack.push(view_trans);
		set_color(0.64f, 0.17f, 0.17f);
		view_trans *= Scale(1.61, 10, 1.61);
		view_trans *= RotateX(90);
		drawCylinder();
		view_trans = mvstack.top(); mvstack.pop();


		set_color(0.16, 0.73, 0.16);
		view_trans *= Translate(0, 11, 0);
		view_trans *= Scale(8, 5, 7);
		drawSphere();

	}
	else if (treeN == 1)
	{
		set_color(0.61f, 0.21f, 0.181f);
		mvstack.push(view_trans);
		view_trans *= RotateY(110);
		view_trans *= Scale(3, 8, 3);
		view_trans *= RotateX(90);
		drawCone();
		view_trans = mvstack.top(); mvstack.pop();

		view_trans *= Translate(0, 2, 0);
		view_trans *= RotateX(6);
		mvstack.push(view_trans);
		set_color(0.64f, 0.17f, 0.17f);
		view_trans *= Scale(1.61, 10, 1.61);
		view_trans *= RotateX(90);
		drawCylinder();
		view_trans = mvstack.top(); mvstack.pop();


		set_color(.31, .72, 0.43);
		view_trans *= Translate(0, 11, 0);
		view_trans *= Scale(8, 5, 7);
		drawSphere();
	}
	else //default tree
	{

	}
	view_trans = mvstack.top(); mvstack.pop();
}
int basis_id = 0;
void drawOneAxis()
{
	mat4 origin = view_trans;
	view_trans *= Translate	( 0, 0, 4 );
	view_trans *= Scale(.25) * Scale( 1, 1, -1 );
	drawCone();
	view_trans = origin;
	view_trans *= Translate	( 1,  1, .5 );
	view_trans *= Scale		( .1, .1, 1 );
	drawCube();
	view_trans = origin;
	view_trans *= Translate	( 1, 0, .5 );
	view_trans *= Scale		( .1, .1, 1 );
	drawCube();
	view_trans = origin;
	view_trans *= Translate	( 0,  1, .5 );
	view_trans *= Scale		( .1, .1, 1 );
	drawCube();
	view_trans = origin;
	view_trans *= Translate	( 0,  0, 2 );
	view_trans *= Scale(.1) * Scale(   1, 1, 20);
    drawCylinder();	
	view_trans = origin;
}

void drawAxes(int selected)
{
	if( basis_to_display != selected ) 
		return;
	mat4 given_basis = view_trans;
	view_trans *= Scale		(.25);
	drawSphere();
	view_trans = given_basis;
	set_color( 0, 0, 1 );
	drawOneAxis();
	view_trans *= RotateX	(-90);
	view_trans *= Scale		(1, -1, 1);
	set_color( 1, 1, 1);
	drawOneAxis();
	view_trans = given_basis;
	view_trans *= RotateY	(90);
	view_trans *= Scale		(-1, 1, 1);
	set_color( 1, 0, 0 );
	drawOneAxis();
	view_trans = given_basis;
	
	colors.pop();
	colors.pop();
	colors.pop();
	set_color( colors.top().r, colors.top().g, colors.top().b );
}
void drawFlower();
void drawGround(){
	
	//ground
	mvstack.push(view_trans);
    set_color( 1,1,1 );
    view_trans *= Translate	(0, -10, 0);									drawAxes(basis_id++);
    view_trans *= Scale		(700, 1, 700);									drawAxes(basis_id++);
    drawGroundCube();
	view_trans = mvstack.top(); mvstack.pop();	drawAxes(basis_id++);
	//markers
	//flower
	mvstack.push(view_trans);

	view_trans *= Translate(-20, -9, -5); drawFlower();
	for (int i = 3; i < 7; i++)
		for (int j = 3; j < 9; j++)
		{
			mvstack.push(view_trans);
			view_trans *= Translate(31*(i + -(i % 2) * i) + (i%4)*3, 0, j*7 + j*14*(j%2));
			drawFlower();
			view_trans = mvstack.top(); mvstack.pop();
		}
	view_trans = mvstack.top(); mvstack.pop();

	//trees
	mvstack.push(view_trans);

	view_trans *= Translate(31, 11.5, 31);
	view_trans *= Scale(2, 3, 2);
	drawTree(0);

	view_trans *= Translate(12, 0, 15);
	drawTree(1);

	view_trans *= Translate(-40, 0, 20);
	drawTree(0);

	view_trans *= Translate(-20, 0, 10);
	drawTree(0);

	view_trans *= Translate(-20, 0, -130);
	drawTree(1);

	view_trans *= Translate(10, 0, 30);
	drawTree(0);
	view_trans *= Translate(-25, 0, 35); drawTree(0);
	view_trans *= Translate(40, 0, 5); //drawTree(0);
	view_trans *= Translate(50, 0, -70); drawTree(1);
	view_trans *= Translate(20, 0, 30); drawTree(0);
    view_trans *= Translate(-10, 0, 40); drawTree(0);
	view_trans *= Translate(-10, 0, 120); drawTree(0);
	view_trans *= Translate(60, 0, 40); drawTree(0);
	view_trans *= Translate(30, 0, -20); drawTree(1);
	view_trans *= Translate(-10, 0, -200); drawTree(0);
	view_trans *= Translate(-10, 0, 40); drawTree(0);
	view_trans = mvstack.top(); mvstack.pop();

	
	//sky
	set_color(1, 1, 1);
	mvstack.push(view_trans);
	view_trans *= Translate(0, -50, 0);
	view_trans *= RotateX(-90);
	view_trans *= Scale(500, 500, 500);
	drawSkySphere();

	//drawSphere();
	view_trans = mvstack.top(); mvstack.pop();
	
}


void drawShapes()
{
	mvstack.push(view_trans);

    view_trans *= Translate	( 0, 3, 0 );									drawAxes(basis_id++);
    view_trans *= Scale		( 3, 3, 3 );									drawAxes(basis_id++);
    set_color( .8, .0, .8 );
    drawCube();

    view_trans *= Scale		( 1/3.0f, 1/3.0f, 1/3.0f );						drawAxes(basis_id++);
    view_trans *= Translate	( 0, 3, 0 );									drawAxes(basis_id++);
    set_color( 0, 1, 0 );
    drawCone();

    view_trans *= Translate	( 0, -9, 0 );									drawAxes(basis_id++);
    set_color( 1, 1, 0 );
    drawCylinder();

	view_trans = mvstack.top(); mvstack.pop();								drawAxes(basis_id++);
	
    view_trans *= Scale		( 1/3.0f, 1/3.0f, 1/3.0f );						drawAxes(basis_id++);

	drawGround();
}

void drawHair()
{
	mvstack.push(view_trans);
	for (int i = 0; i < 5; i++)
	{
		view_trans *= Scale(0.83);
		view_trans *= RotateX(-10*i+i*i);
		view_trans *= Translate(0, 0, 1.8);
		drawSphere();
	}
	
	view_trans = mvstack.top(); mvstack.pop();
}

void drawPersonHead( float tilt) // around 5 high?
{
	mvstack.push(view_trans);
	//neck
	view_trans *= RotateX(tilt);
	view_trans *= Translate(0, 0.3, 0);
	mvstack.push(view_trans);
	view_trans *= Scale(0.8, 1, 0.8);
	view_trans *= RotateX(90);
	 
	drawCylinder();
	view_trans = mvstack.top(); mvstack.pop();


	//face
	view_trans *= Translate(0, 2.5, 0);
	view_trans *= Scale(1.8, 2, 1.8);

	 
	drawSphere();
	view_trans = mvstack.top(); mvstack.pop();
}

void drawPersonLeg(float hipA, float hipB, float kneeA)
{
	mvstack.push(view_trans);
	//top
	view_trans *= RotateX(hipA);
	view_trans *= RotateY(hipB);
	 
	drawSphere();
	//upper leg
	view_trans *= Translate(0, 0, 2.5);
	mvstack.push(view_trans);
	view_trans *= Scale(1, 1, 5);
	 
	drawCube();
	view_trans = mvstack.top(); mvstack.pop();
	//knee
	view_trans *= Translate(0, 0, 2.5);
	mvstack.push(view_trans);
	view_trans *= Scale(0.8);
	 
	drawSphere();
	view_trans = mvstack.top(); mvstack.pop();
	//lower leg
	view_trans *= RotateX(kneeA);
	view_trans *= Translate(0, 0, 2.5);
	mvstack.push(view_trans);
	view_trans *= Scale(1, 1, 5);
	 
	drawCube();
	view_trans = mvstack.top(); mvstack.pop();
	//"foot"
	view_trans *= Translate(0, 0, 2.5);
	 
	drawSphere();

	view_trans = mvstack.top(); mvstack.pop();


}
void drawFlower()
{
	mvstack.push(view_trans);

	view_trans *= Scale(0.12, 2, 0.12);
	view_trans *= RotateX(90);	
	set_color(0, 0.7, 0.2);
	drawCylinder();
	view_trans = mvstack.top(); mvstack.pop();

	
	mvstack.push(view_trans);
	view_trans *= Translate(0, 2, 0);
	set_color(.85, .1, 0);
	view_trans *= Scale(1, 0.2, 1);
	drawSphere();
	view_trans = mvstack.top(); mvstack.pop();

	mvstack.push(view_trans);
	view_trans *= Translate(0, 2, 0);
	view_trans *= Scale(0.5, 0.1, 0.5);
	set_color(.78,.12,.12);
	for (int i = 0; i < 6; i++)
	{
		
		view_trans *= RotateY(60);
		view_trans *= Translate(2.2, 0, 0);
		drawSphere();
		view_trans *= Translate(-2.2, 0, 0);
	}
	view_trans = mvstack.top(); mvstack.pop();


}

void drawPersonFlower(float shoulderA, int shoulderB, int elbowA)
{
	//shoulder

	drawSphere();


	//upper arm
	mvstack.push(view_trans);


	view_trans *= RotateY(90);
	view_trans *= RotateZ(shoulderA);
	view_trans *= RotateX(shoulderB);

	view_trans *= Translate(0, 0, 2);

	mvstack.push(view_trans);
	view_trans *= Scale(1, 1, 4);

	drawCube();

	view_trans = mvstack.top(); mvstack.pop();

	//elbow
	view_trans *= Translate(0, 0, 2);
	mvstack.push(view_trans);
	view_trans *= Scale(0.8);

	drawSphere();

	view_trans = mvstack.top(); mvstack.pop();
	//forearm
	view_trans *= RotateY(elbowA);
	view_trans *= RotateX(30);
	view_trans *= Translate(0, 0, 2);
	mvstack.push(view_trans);
	view_trans *= Scale(1, 1, 4);

	drawCube();

	view_trans = mvstack.top(); mvstack.pop();
	view_trans *= Translate(0, 0, 2);
	//hand
	mvstack.push(view_trans);
	view_trans *= Scale(0.6);
	drawSphere();
	view_trans = mvstack.top(); mvstack.pop();

	view_trans *= RotateZ(90);
	view_trans *= RotateX(0);
	view_trans *= Translate(0, -1, 0);
	mvstack.push(view_trans);
	view_trans *= RotateX(160);
	drawFlower();
	view_trans = mvstack.top(); mvstack.pop();




	view_trans = mvstack.top(); mvstack.pop();
}

void drawPersonCane(float shoulderA, float shoulderB, float elbowA)
{


	//shoulder

	drawSphere();


	//upper arm
	mvstack.push(view_trans);


	view_trans *= RotateY(90);
	view_trans *= RotateZ(shoulderA);
	view_trans *= RotateX(shoulderB);

	view_trans *= Translate(0, 0, 2);

	mvstack.push(view_trans);
	view_trans *= Scale(1, 1, 4);

	drawCube();

	view_trans = mvstack.top(); mvstack.pop();

	//elbow
	view_trans *= Translate(0, 0, 2);
	mvstack.push(view_trans);
	view_trans *= Scale(0.8);

	drawSphere();

	view_trans = mvstack.top(); mvstack.pop();
	//forearm
	view_trans *= RotateY(elbowA);
	view_trans *= RotateX(30);
	view_trans *= Translate(0, 0, 2);
	mvstack.push(view_trans);
	view_trans *= Scale(1, 1, 4);

	drawCube();

	view_trans = mvstack.top(); mvstack.pop();
	view_trans *= Translate(0, 0, 2);
	//hand
	mvstack.push(view_trans);
	view_trans *= Scale(0.6);
	drawSphere(); 
	view_trans = mvstack.top(); mvstack.pop();

	view_trans *= RotateZ(90);
	view_trans *= RotateX(30);
	view_trans *= Translate(0, 4, 0);
	mvstack.push(view_trans);
	view_trans *= RotateX(90);
	view_trans *= Scale(.5, .5, 4);
	set_color(0.6, 0.7, 0.3);
	drawCylinder();
	view_trans = mvstack.top(); mvstack.pop();




	view_trans = mvstack.top(); mvstack.pop();
}
void drawPersonArm( float shoulderA, float shoulderB, float elbowA)
{


	//shoulder
	 
	drawSphere();


	//upper arm
	mvstack.push(view_trans);


	view_trans *= RotateY(90);
	view_trans *= RotateZ(shoulderA);
	view_trans *= RotateX(shoulderB);

	view_trans *= Translate(0, 0, 2);

	mvstack.push(view_trans);
	view_trans *= Scale(1, 1, 4);
	 
	drawCube();

	view_trans = mvstack.top(); mvstack.pop();

	//elbow
	view_trans *= Translate(0, 0, 2);
	mvstack.push(view_trans);
	view_trans *= Scale(0.8);
	 
	drawSphere();

	view_trans = mvstack.top(); mvstack.pop();
	//forearm
	view_trans *= RotateY(elbowA);
	view_trans *= RotateX(30);
	view_trans *= Translate(0, 0, 2);
	mvstack.push(view_trans);
	view_trans *= Scale(1, 1, 4);
	 
	drawCube();

	view_trans = mvstack.top(); mvstack.pop();
	view_trans *= Translate(0, 0, 2);
	//hand

	view_trans = mvstack.top(); mvstack.pop();
}

void drawPersonWalking2()
{
	float personTime = getTime(PROPOSETIME+10, 20, 9999);
	mvstack.push(view_trans);
	view_trans *= Translate(0, -8, 0);

	set_color(0.3, 0.5, 0.5);

	//left leg
	mvstack.push(view_trans);
	view_trans *= Translate(-3, 0, 0);
	drawPersonLeg(70 - 30 * sin(-1.3*personTime), 0, 20 + sin(-personTime) * 15);
	view_trans = mvstack.top(); mvstack.pop();

	//right leg
	mvstack.push(view_trans);
	view_trans *= Translate(3, 0, 0);
	drawPersonLeg(70 - 30 * sin(1.3*personTime), 0, 20 + sin(personTime) * 15);
	view_trans = mvstack.top(); mvstack.pop();


	view_trans *= RotateX(20 + 8 * sin(2.6 * personTime - 0.5));
	//right arm
	mvstack.push(view_trans);
	view_trans *= Translate(3, 8, 0);
	drawPersonArm(9 * sin(personTime), 70, -30);
	view_trans = mvstack.top(); mvstack.pop();

	//left arm
	mvstack.push(view_trans);
	view_trans *= Translate(-3, 8, 0);
	view_trans *= RotateY(180);
	drawPersonArm(8 * sin(personTime), 70, 30);
	view_trans = mvstack.top(); mvstack.pop();

	set_color(0.5, 0.6, 0.7);

	//body
	mvstack.push(view_trans);
	view_trans *= Translate(0, 4, 0);
	view_trans *= Scale(6, 8, 2);

	drawCube();
	view_trans = mvstack.top(); mvstack.pop();

	//head
	set_color(0.2, 0.4, 0.5);
	mvstack.push(view_trans);
	view_trans *= Translate(0, 8, 0);
	drawPersonHead(30);
	view_trans = mvstack.top(); mvstack.pop();


	view_trans = mvstack.top(); mvstack.pop();
}


void drawPersonWalking()
{
	float personTime = getTime(0, INVERSETIME-3, INVERSETIME);
	mvstack.push(view_trans);
	view_trans *= Translate(0, -8, 0);

	set_color(0.3, 0.5, 0.5);
	
	//left leg
	mvstack.push(view_trans);
	view_trans *= Translate(-3, 0, 0);
	drawPersonLeg(70 - 30 * sin(-1.3*personTime), 0, 20 + sin(-personTime) * 15);
	view_trans = mvstack.top(); mvstack.pop();

	//right leg
	mvstack.push(view_trans);
	view_trans *= Translate(3, 0, 0);
	drawPersonLeg(70 - 30 * sin(1.3*personTime), 0, 20 + sin(personTime) * 15);
	view_trans = mvstack.top(); mvstack.pop();


	view_trans *= RotateX(20 + 8 * sin(2.6 * personTime - 0.5));
	//right arm
	mvstack.push(view_trans);
	view_trans *= Translate(3, 8, 0);
	drawPersonArm(9 * sin(personTime), 70, -30);
	view_trans = mvstack.top(); mvstack.pop();

	//left arm
	mvstack.push(view_trans);
	view_trans *= Translate(-3, 8, 0);
	view_trans *= RotateY(180);
	drawPersonArm(8 * sin(personTime), 70, 30);
	view_trans = mvstack.top(); mvstack.pop();
	
	set_color(0.5, 0.6, 0.7);
	
	//body
	mvstack.push(view_trans);
	view_trans *= Translate(0, 4, 0);
	view_trans *= Scale(6, 8, 2);
	 
	drawCube();
	view_trans = mvstack.top(); mvstack.pop();

	//head
	set_color(0.2, 0.4, 0.5);
	mvstack.push(view_trans);
	view_trans *= Translate(0, 8, 0);
	drawPersonHead(  30);
	view_trans = mvstack.top(); mvstack.pop();


	view_trans = mvstack.top(); mvstack.pop();
}

void drawHiGirl()
{
	float tempTime;
	tempTime = getTime(SOCCERTIME+8,4.5, INVERSETIME*1.5-(SOCCERTIME+12.5f)/2.0f);

	mvstack.push(view_trans);
	view_trans *= Translate(0, -8, 0);

	//left leg
	set_color(0.63f, 0.2f, 0.51f);
	mvstack.push(view_trans);
	view_trans *= Translate(-3, 0, 0);
	drawPersonLeg(85, 0, 15);
	view_trans = mvstack.top(); mvstack.pop();

	//right leg
	mvstack.push(view_trans);
	view_trans *= Translate(3, 0, 0);
	drawPersonLeg(90, 0, 10);
	view_trans = mvstack.top(); mvstack.pop();


	view_trans *= RotateX(0);
	//right arm
	mvstack.push(view_trans);
	view_trans *= Translate(3, 8, 0);
	view_trans *= RotateY(180);
	drawPersonArm(0, 180+68 * sin(5*tempTime + 0.3), 0);
	view_trans = mvstack.top(); mvstack.pop();

	//left arm
	mvstack.push(view_trans);
	view_trans *= Translate(-3, 8, 0);
	//view_trans *= RotateY(180);
	drawPersonArm(0, 180+70 * sin(5*tempTime + 0.3), 0);
	view_trans = mvstack.top(); mvstack.pop();

	//body
	set_color(0.52f, 0.43f, 0.62f);
	mvstack.push(view_trans);
	view_trans *= Translate(0, 4, 0);
	view_trans *= Scale(6, 8, 2);

	drawCube();
	view_trans = mvstack.top(); mvstack.pop();

	//head
	set_color(0.4752f, 0.47f, 0.62f);
	mvstack.push(view_trans);
	view_trans *= Translate(0, 8, 0);
	drawPersonHead(10);
	view_trans = mvstack.top(); mvstack.pop();
	//draw hair
	set_color(0.13f, 0.1f, 0.1f);
	mvstack.push(view_trans);
	view_trans *= Translate(-1.3, 13, 0);
	view_trans *= RotateY(90);
	view_trans *= RotateX(130);
	view_trans *= Scale(0.7);
	drawHair();
	view_trans = mvstack.top(); mvstack.pop();

	//other hair
	mvstack.push(view_trans);
	view_trans *= Translate(1.3, 13, 0);
	view_trans *= RotateY(-90);
	view_trans *= RotateX(130);
	view_trans *= Scale(0.7);
	drawHair();
	view_trans = mvstack.top(); mvstack.pop();

	view_trans = mvstack.top(); mvstack.pop();

}
void drawKickingGirl()
{
	//time control
	float tempTime = getTime(SOCCERTIME+5-2,4,INVERSETIME*1.5 -(SOCCERTIME+7)/2.0f);

	//end time control

	mvstack.push(view_trans);
	view_trans *= Translate(0, -8, 0);

	//left leg
	set_color(0.63f, 0.2f, 0.51f);
	mvstack.push(view_trans);
	view_trans *= Translate(-3, 0, 0);
	drawPersonLeg(90 + 30 * sin(2*tempTime), 0, 35 + 30 * sin(tempTime));
	view_trans = mvstack.top(); mvstack.pop();

	//right leg
	mvstack.push(view_trans);
	view_trans *= Translate(3, 0, 0);
	drawPersonLeg(90, 0, 0);
	view_trans = mvstack.top(); mvstack.pop();


	view_trans *= RotateX(0);
	//right arm
	mvstack.push(view_trans);
	view_trans *= Translate(3, 8, 0);
	drawPersonArm(15 + 25 * sin(2*tempTime + 0.3), 65, -25);
	view_trans = mvstack.top(); mvstack.pop();

	//left arm
	mvstack.push(view_trans);
	view_trans *= Translate(-3, 8, 0);
	view_trans *= RotateY(180);
	drawPersonArm(-10 + 25 * sin(2*tempTime + 0.8), 70, 30);
	view_trans = mvstack.top(); mvstack.pop();

	//body
	set_color(0.52f, 0.43f, 0.62f);
	mvstack.push(view_trans);
	view_trans *= Translate(0, 4, 0);
	view_trans *= Scale(6, 8, 2);

	drawCube();
	view_trans = mvstack.top(); mvstack.pop();

	//head
	set_color(0.4752f, 0.47f, 0.62f);
	mvstack.push(view_trans);
	view_trans *= Translate(0, 8, 0);
	drawPersonHead(10);
	view_trans = mvstack.top(); mvstack.pop();
	//draw hair
	set_color(0.13f, 0.1f, 0.1f);
	mvstack.push(view_trans);
	view_trans *= Translate(-1.3, 13, 0);
	view_trans *= RotateY(90);
	view_trans *= RotateX(130);
	view_trans *= Scale(0.7);
	drawHair();
	view_trans = mvstack.top(); mvstack.pop();
	
	//other hair
	mvstack.push(view_trans);
	view_trans *= Translate(1.3, 13, 0);
	view_trans *= RotateY(-90);
	view_trans *= RotateX(130);
	view_trans *= Scale(0.7);
	drawHair();
	view_trans = mvstack.top(); mvstack.pop();

	view_trans = mvstack.top(); mvstack.pop();
}

void drawKickingPerson()
{
	float tempTime = getTime(SOCCERTIME-2, 4, INVERSETIME*1.5-(SOCCERTIME+2)/2.0f);

	mvstack.push(view_trans);
	view_trans *= Translate(0, -8, 0);

	set_color(0.16f, 0.16f, 0.43f);
	//left leg
	mvstack.push(view_trans);
	view_trans *= Translate(-3, 0, 0);

	drawPersonLeg(90 + 30*sin(2*tempTime), 0, 35 + 30*sin(2*tempTime));
	view_trans = mvstack.top(); mvstack.pop();
	
	//right leg
	mvstack.push(view_trans);
	view_trans *= Translate(3, 0, 0);
	drawPersonLeg(90, 0, 0);
	view_trans = mvstack.top(); mvstack.pop();


	view_trans *= RotateX(0);
	//right arm
	mvstack.push(view_trans);
	view_trans *= Translate(3, 8, 0);
	drawPersonArm(15 + 35*sin(2*tempTime + 0.3), 65, -25);
	view_trans = mvstack.top(); mvstack.pop();

	//left arm
	mvstack.push(view_trans);
	view_trans *= Translate(-3, 8, 0);
	view_trans *= RotateY(180);
	drawPersonArm(-10 + 35*sin(2*tempTime + 0.8),70, 30);
	view_trans = mvstack.top(); mvstack.pop();

	//body
	mvstack.push(view_trans);
	view_trans *= Translate(0, 4, 0);
	view_trans *= Scale(6, 8, 2);

	drawCube();
	view_trans = mvstack.top(); mvstack.pop();

	//head
	set_color(0.3f, 0.3f, 0.41f);
	mvstack.push(view_trans);
	view_trans *= Translate(0, 8, 0);
	drawPersonHead(10);
	view_trans = mvstack.top(); mvstack.pop();


	view_trans = mvstack.top(); mvstack.pop();
}


void drawGrandpaWalking()
{

	float walkTime = getTime(0, GRANDPATIME, INVERSETIME*1.5F - (GRANDPATIME)/2.0f);
	float fallTime = getTime(GRANDPATIME, 1.5, INVERSETIME*1.5f - (GRANDPATIME+1.5)/2.0f);
	mvstack.push(view_trans);
	set_color(0.2, 0.3, 0.2);
	view_trans *= Translate(0, -8, walkTime*1.5); //walk across Z
	if (fallTime > 0)
	{
		view_trans *= Translate(0, -9, 0);
		view_trans *= RotateX(-80/3*fallTime*2);
		view_trans *= Translate(0, 9, 0);
	}
	//left leg
	mvstack.push(view_trans);
	view_trans *= Translate(-3, 0, 0);

	drawPersonLeg(70 - 30 * sin(.9*walkTime)
			,0
			,20+10*sin(-walkTime));
	view_trans = mvstack.top(); mvstack.pop();

	//right leg
	mvstack.push(view_trans);
	view_trans *= Translate(3, 0, 0);
	drawPersonLeg(70-30*sin(-.9*walkTime),
		0,
		20+sin(walkTime)*15);
	view_trans = mvstack.top(); mvstack.pop();


	view_trans *= RotateX(40+10*sin(walkTime));
	//left arm
	mvstack.push(view_trans);
	view_trans *= Translate(3, 8, 0);
	view_trans *= RotateY(30);
	drawPersonArm(40,70,-50);
	view_trans = mvstack.top(); mvstack.pop();

	//right arm
	mvstack.push(view_trans);
	view_trans *= Translate(-3, 8, 0);
	view_trans *= RotateY(180);
	
	drawPersonCane(60, 70, 20 + 15 * sin(.9*walkTime));
	view_trans = mvstack.top(); mvstack.pop();

	//body
	set_color(0.2, 0.3, 0.2);
	mvstack.push(view_trans);
	view_trans *= Translate(0, 4, 0);
	view_trans *= Scale(6, 8, 2);

	drawCube();
	view_trans = mvstack.top(); mvstack.pop();

	//head
	mvstack.push(view_trans);
	view_trans *= Translate(0, 8, 0);
	drawPersonHead(10);
	view_trans = mvstack.top(); mvstack.pop();


	view_trans = mvstack.top(); mvstack.pop();
}

void drawTheBoy()
{
	float Time = getTime(PROPOSETIME, 7, 999);
	mvstack.push(view_trans);
	view_trans *= Translate(0, -8, 0);

	set_color(0.3, 0.5, 0.5);
	//left leg
	mvstack.push(view_trans);
	view_trans *= Translate(-3, 0, 0);

	drawPersonLeg(90
		, 0
		, 0);
	view_trans = mvstack.top(); mvstack.pop();

	//right leg
	mvstack.push(view_trans);
	view_trans *= Translate(3, 0, 0);
	drawPersonLeg(95,
		0,
		0);
	view_trans = mvstack.top(); mvstack.pop();


	//left arm
	mvstack.push(view_trans);
	view_trans *= Translate(3, 8, 0);
	view_trans *= RotateY(30);
	drawPersonArm(0, 70, 0);
	view_trans = mvstack.top(); mvstack.pop();

	//right arm
	mvstack.push(view_trans);
	view_trans *= Translate(-3, 8, 0);
	view_trans *= RotateY(180);
	drawPersonFlower(10.0f*Time, 70, 15);
	view_trans = mvstack.top(); mvstack.pop();

	set_color(0.5, 0.6, 0.7);
	//body

	mvstack.push(view_trans);
	view_trans *= Translate(0, 4, 0);
	view_trans *= Scale(6, 8, 2);

	drawCube();
	view_trans = mvstack.top(); mvstack.pop();

	//head
	set_color(0.2, 0.4, 0.5);
	mvstack.push(view_trans);
	view_trans *= Translate(0, 8, 0);
	drawPersonHead(10);
	view_trans = mvstack.top(); mvstack.pop();


	view_trans = mvstack.top(); mvstack.pop();
}


void drawTheGirl()
{
	//wait

	//look down at flower

	//look up to him

	//shake face

	//walk away
	//time control
	float girlTime = getTime(PROPOSETIME + 7, 3, 9999);

	//end time control

	mvstack.push(view_trans);
	view_trans *= Translate(0, -8, 0);

	//left leg
	set_color(0.63f, 0.2f, 0.51f);
	mvstack.push(view_trans);
	view_trans *= Translate(-3, 0, 0);
	drawPersonLeg(90 , 0, 0);
	view_trans = mvstack.top(); mvstack.pop();

	//right leg
	mvstack.push(view_trans);
	view_trans *= Translate(3, 0, 0);
	drawPersonLeg(90, 0, 0);
	view_trans = mvstack.top(); mvstack.pop();


	view_trans *= RotateX(0);
	//right arm
	mvstack.push(view_trans);
	view_trans *= Translate(3, 8, 0);
	drawPersonArm(0, 65, -25);
	view_trans = mvstack.top(); mvstack.pop();

	//left arm
	mvstack.push(view_trans);
	view_trans *= Translate(-3, 8, 0);
	view_trans *= RotateY(180);
	drawPersonArm(0, 70, 30);
	view_trans = mvstack.top(); mvstack.pop();

	//body
	set_color(0.52f, 0.43f, 0.62f);
	mvstack.push(view_trans);
	view_trans *= Translate(0, 4, 0);
	view_trans *= Scale(6, 8, 2);

	drawCube();
	view_trans = mvstack.top(); mvstack.pop();

	//head
	set_color(0.4752f, 0.47f, 0.62f);
	view_trans *= RotateY(20 * sin(2*girlTime));
	mvstack.push(view_trans);
	view_trans *= Translate(0, 8, 0);
	drawPersonHead(10);
	view_trans = mvstack.top(); mvstack.pop();
	//draw hair
	set_color(0.3f, 0.4f, 0.2f);
	mvstack.push(view_trans);
	view_trans *= Translate(-1, 13, 0);
	view_trans *= RotateY(90);
	view_trans *= RotateX(130);
	view_trans *= Scale(0.7);
	drawHair();
	view_trans = mvstack.top(); mvstack.pop();

	//other hair
	mvstack.push(view_trans);
	view_trans *= Translate(1, 13, 0);
	view_trans *= RotateY(-90);
	view_trans *= RotateX(130);
	view_trans *= Scale(0.7);
	drawHair();
	view_trans = mvstack.top(); mvstack.pop();

	view_trans = mvstack.top(); mvstack.pop();
}
void drawLeafFalling2(float beginTime);
void drawProposalScene()
{
	mvstack.push(view_trans);
	
	if (TIME < PROPOSETIME + 10)
		drawTheBoy();
	else
	{
		
		mvstack.push(view_trans);
		view_trans *= Translate(0, 0, -2*(TIME - (INVERSETIME*1.5 + 10)));
		view_trans *= RotateY(180);
		drawPersonWalking2();
		view_trans = mvstack.top(); mvstack.pop();
	}
	view_trans *= Translate(0, 0, 15);
	view_trans *= RotateY(180);
	drawTheGirl();
	view_trans *= Translate(12, 0, 10);
	drawLeafFalling2(INVERSETIME*1.5);

	view_trans = mvstack.top(); mvstack.pop();
}

void drawSoccerScene() // soccer for 3 seconds
{
	float beginTime, duration, reverseTime;
	float KickTime = getTime(SOCCERTIME, 3, INVERSETIME*1.5 - (SOCCERTIME + 3.0) / 2.0f);
	float secondKickTime = getTime(SOCCERTIME + 5, 3, INVERSETIME*1.5 - (SOCCERTIME + 5 + 3.0) / 2.0f);
	float lastKickTime = getTime(SOCCERTIME + 12, 3, INVERSETIME*1.5 - (SOCCERTIME + 12 + 3.0) / 2.0f);
	mvstack.push(view_trans);
	
	if (KickTime > 0)
		view_trans *= Translate(9.f*KickTime, 3 * sin(KickTime * 60 * DegreesToRadians), -KickTime*(4.0 / 3.0));
	if (secondKickTime > 0)
		view_trans *= Translate( -17.0f*secondKickTime, 8 * sin(secondKickTime * 60 * DegreesToRadians), 3* secondKickTime);
	if (lastKickTime > 0)
		view_trans *= Translate(3.0f*lastKickTime, 3 * sin(lastKickTime * 60 * DegreesToRadians), 4.f * lastKickTime);
	view_trans *= Translate(2, -17, 2 );
	
	drawSphere();
	view_trans = mvstack.top(); mvstack.pop();

	mvstack.push(view_trans);
	view_trans *= RotateY(90);
	drawKickingPerson();
	view_trans = mvstack.top(); mvstack.pop();

	mvstack.push(view_trans);
	view_trans *= Translate(30, 0, 0);
	view_trans *= RotateY(-90);
	//WHICH GIRL DO I DRAW?
	if (TIME > SOCCERTIME + 8 && TIME < (INVERSETIME*1.5f - (SOCCERTIME + 11.0f) / 2.0f) )
		drawHiGirl();
	else
		drawKickingGirl();
	view_trans = mvstack.top(); mvstack.pop();
}

std::stack<mat4> leafPlace;

void drawFallingLeaf(float seed, float leafTime)
{
	mvstack.push(view_trans);

	view_trans *= Translate(0, -(1.3*leafTime+seed/3.0f), 0);
	view_trans *= RotateZ(80*(leafTime+(seed*3)+seed/3));
	view_trans *= Scale(2.5);
	set_color(0.2, 0.6, 0.3);
	drawLeaf();

	view_trans = mvstack.top(); mvstack.pop();

}

void drawLeafFalling2(float beginTime)
{
	float leafTime = getTime(beginTime, 40, 99999);
	for (int i = 0; i < 10; i++)
	{
		mvstack.push(view_trans);
		view_trans *= Translate(-30.0f + 13 * (i % 3), 15 - (i*i - 6 * i), -20.0f - (i*i - 7 * i));
		drawFallingLeaf(i, leafTime);
		view_trans = mvstack.top(); mvstack.pop();
	}

}

void drawLeafFalling(float beginTime)
{
	float leafTime = getTime(beginTime, INVERSETIME-beginTime, INVERSETIME+beginTime);
	for (int i = 0; i < 10; i++)
	{
		mvstack.push(view_trans);
		view_trans *= Translate(-30.0f + 13 * (i % 3), 15 - (i*i - 6 * i), -20.0f - (i*i - 7 * i));
		drawFallingLeaf(i, leafTime);
		view_trans = mvstack.top(); mvstack.pop();
	}
	
}

/*
void drawPerson( )
{
	mvstack.push(view_trans);


	drawPersonBody(  20 + 8 * sin(2.6 * TIME - 0.5));

	//drawPersonLeg(view_trans, view_trans);
	//drawPersonLeg(view_trans, view_trans);
	view_trans = mvstack.top(); mvstack.pop();
}*/

void drawPlanets()
{
    set_color( .8, .0, .0 );	//model sun
    mvstack.push(view_trans);
    view_trans *= Scale(3);													drawAxes(basis_id++);
    drawSphere();
    view_trans = mvstack.top(); mvstack.pop();								drawAxes(basis_id++);
    
    set_color( .0, .0, .8 );	//model earth
    view_trans *= RotateY	( 10*TIME );									drawAxes(basis_id++);
    view_trans *= Translate	( 15, 5*sin( 30*DegreesToRadians*TIME ), 0 );	drawAxes(basis_id++);
    mvstack.push(view_trans);
    view_trans *= RotateY( 300*TIME );										drawAxes(basis_id++);
    drawCube();
    view_trans = mvstack.top(); mvstack.pop();								drawAxes(basis_id++);
    
    set_color( .8, .0, .8 );	//model moon
    view_trans *= RotateY	( 30*TIME );									drawAxes(basis_id++);
    view_trans *= Translate	( 2, 0, 0);										drawAxes(basis_id++);
    view_trans *= Scale(0.2);												drawAxes(basis_id++);
    drawCylinder();
	
}

void drawMidterm()
{
	mvstack.push(view_trans);
	mvstack.push(view_trans);
	view_trans *= Translate	( -1, 0, 0 );									drawAxes(basis_id++);
	view_trans *= Scale		( 2, 1, 1 );									drawAxes(basis_id++);
	drawCube();
	view_trans = mvstack.top(); mvstack.pop();								drawAxes(basis_id++);
	
	view_trans *= Scale		( 2, 1, 1 );									drawAxes(basis_id++);
	view_trans *= Translate	( 1, 0, 0 );									drawAxes(basis_id++);
	drawCube();

	
	view_trans *= Translate	( 0, 2, 0 );									drawAxes(basis_id++);
	view_trans *= RotateZ	( 90 + 360 * TIME );							drawAxes(basis_id++);
	drawCube();
	view_trans = mvstack.top(); mvstack.pop();								drawAxes(basis_id++);
}

int leafCount = 0;

mat4 cameraControl()
{
	mat4 cc;
	float cameraTime = getTime(0, INVERSETIME, INVERSETIME);
	vec4 tempeye, tempref;
	if (cameraTime > 0 && cameraTime < 5.5)
	{
		tempeye  = Translate(-45, -3 + 2*cameraTime, -72 - 2 * cameraTime) * eye;
		tempref = Translate(-30, -5 + 2*cameraTime, -70 + 3 * cameraTime) * ref;
	}
	else if (cameraTime > 5.5 && cameraTime < 10)
	{
		tempeye = Translate(-45, -3 + 2 * 5.5, -72 - 2 * 5.5 + 3*(cameraTime-5.5)) * eye;
		tempref = Translate(-30, -5 + 2 * 5.5, -70 + 3 * cameraTime) * ref;
	}
	else if (cameraTime > 10 && cameraTime < 12)
	{
		tempeye = Translate(65, 13, 15) * eye;
		tempref = ref;
	}
	else if (cameraTime > 12 && cameraTime < INVERSETIME-3)
	{
		tempeye = Translate(-65, -3 + 2 * 5.5, -20 - 2 * 5.5 + 3 * (cameraTime - 5.5)) * eye;
		tempref = Translate(-30, -5 + 2 * 5.5, -50 + 3 * cameraTime) * ref;
	}
	else if (cameraTime > INVERSETIME - 3)
	{
		tempeye = RotateY(90*(TIME-INVERSETIME+3))*Translate(-65, -3 + 2 * 5.5, -20 - 2 * 5.5 + 3 * (INVERSETIME-3 - 5.5)) * eye;
		tempref = Translate(-30, -5 + 2 * 5.5, -50 + 3 * INVERSETIME - 3) * ref;
	}
	//proposal scene
	else if (TIME > INVERSETIME*1.5 && TIME < PROPOSETIME+10)
	{
		tempeye = Translate(10,25,15-1.5*(TIME-INVERSETIME*1.5))*eye;
		tempref = Translate(0,0,10)*ref;
	}
	else if (TIME > PROPOSETIME + 10 && TIME < PROPOSETIME + 20 )
	{
		tempeye = Translate(10 + (TIME-PROPOSETIME-10)*2, 11, 18)* eye;
		tempref = ref;
	}
	cc = LookAt(tempeye, tempref, up);
	return cc;
}
float previousTime = 0;
int frameCount;
void display(void)
{
	basis_id = 0;
    glClearColor( .1, .1, .2, 1 );
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	set_color( .6, .6, .6 );

	float frame = TIME - previousTime;
	previousTime = TIME;	
	frameCount++;
	if ( frameCount%10 ==0 )
	std::cout << "Frame Rate : " << 1 / frame << '\n';

	float personTime;
	personTime = getTime(0, INVERSETIME-3, INVERSETIME);

	view_trans = cameraControl();

	view_trans *= orientation;
    view_trans *= Scale(zoom);												
	
	drawGround();
	view_trans *= Translate(0, 8, 0);

	mvstack.push(view_trans);
	//person stuff here
	if (TIME < INVERSETIME*1.5)
	{
		view_trans *= Translate(-25, 0, -70 + 3 * personTime);
		drawPersonWalking();
		view_trans = mvstack.top(); mvstack.pop();
		//falling leaves


		//view_trans *= Translate(0, 15,0);
		mvstack.push(view_trans);
		view_trans *= Translate(-10, -5, -50);
		drawLeafFalling(0);
		view_trans *= Translate(5, 15, 0);
		drawLeafFalling(0);
		view_trans = mvstack.top(); mvstack.pop();
		drawLeafFalling(0);
		//scenes
		mvstack.push(view_trans);
		view_trans *= RotateY(-30);
		drawSoccerScene();
		view_trans = mvstack.top(); mvstack.pop();

		mvstack.push(view_trans);
		view_trans *= Translate(-27, 0, 65);
		view_trans *= RotateY(180);
		drawGrandpaWalking();
		view_trans = mvstack.top(); mvstack.pop();
		//drawPlanets();
	}
	else //scene 2
	{
		drawProposalScene();
	}
    glutSwapBuffers();
}

void myReshape(int w, int h)	// Handles window sizing and resizing.
{    
    mat4 projection = Perspective( 50, (float)w/h, 1, 1000 );
    glUniformMatrix4fv( uProjection, 1, GL_FALSE, transpose(projection) );
	glViewport(0, 0, g_width = w, g_height = h);	
}		

void instructions() {	 std::cout <<	"Press:"									<< '\n' <<
										"  r to restore the original view."			<< '\n' <<
										"  0 to restore the original state."		<< '\n' <<
										"  a to toggle the animation."				<< '\n' <<
										"  b to show the next basis's axes."		<< '\n' <<
										"  B to show the previous basis's axes."	<< '\n' <<
										"  q to quit."								<< '\n';	}

void myKey(unsigned char key, int x, int y)
{
    switch (key) {
        case 'q':   case 27:				// 27 = esc key
            exit(0); 
		case 'b':
			std::cout << "Basis: " << ++basis_to_display << '\n';
			break;
		case 'B':
			std::cout << "Basis: " << --basis_to_display << '\n';
			break;
        case 'a':							// toggle animation           		
            if(animate) std::cout << "Elapsed time " << TIME << '\n';
            animate = 1 - animate ; 
            break ;
		case '0':							// Add code to reset your object here.
			TIME = 0;	TM.Reset() ;											
        case 'r':
			orientation = mat4();			
            break ;
    }
    glutPostRedisplay() ;
}

int main() 
{
	char title[] = "Title";
	int argcount = 1;	 char* title_ptr = title;
	glutInit(&argcount,		 &title_ptr);
	glutInitWindowPosition (230, 70);
	glutInitWindowSize     (g_width, g_height);
	glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutCreateWindow("Go Back to the Past");
	#if !defined(__APPLE__) && !defined(EMSCRIPTEN)
		glewExperimental = GL_TRUE;
		glewInit();
	#endif
    std::cout << "GL version " << glGetString(GL_VERSION) << '\n';
	instructions();
	init();

	glutDisplayFunc(display);
    glutIdleFunc(idleCallBack) ;
    glutReshapeFunc (myReshape);
    glutKeyboardFunc( myKey );
    glutMouseFunc(myMouseCallBack) ;
    glutMotionFunc(myMotionCallBack) ;
    glutPassiveMotionFunc(myPassiveMotionCallBack) ;

	glutMainLoop();
}