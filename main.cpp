// Include libraries
#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <float.h>
#include "ll_map.h"

using namespace gfox;

// GL Includes
#include <cstdlib>
#include <GL/glut.h>
#include <sys/time.h>

// The Map
ll_map burlington_map;

// Functions for GL delegates
void disp();
void init();
void keyUp(unsigned char, int, int);
void keyDown(unsigned char, int, int);
void reshape(int, int);
void update();

// Window ID
static int win;

// For key input
static bool keyboard[256];

// Camera Stuff
static float distance = 5.0f;
static float rotation = 0.0f;
static float center_x;
static float center_y;
static float center_z;

// All the 'll_map' dumped into a mesh
static float* map_mesh;
static size_t num_map_points;
 
int main(int argc, char* argv[]) {

	glutInit(&argc, argv);
	
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);

	glutInitWindowSize(1280, 800);
	glutInitWindowPosition(0, 0);
	win = glutCreateWindow("ll_map demo");
	glutFullScreen();

	glutDisplayFunc(disp);
	glutKeyboardFunc(keyDown);
	glutKeyboardUpFunc(keyUp);
	glutIdleFunc(update);
	glutReshapeFunc(reshape);

	glClearColor(0.08, 0.08, 0.08, 1.0);

	init();

	glutMainLoop();

	delete[] map_mesh;
	return 0;
}

void init() {

	// Read the elevation data from MapQuest
	burlington_map.build_map(
		44.476221,	// Latitude
		-73.205595,	// Longitude
		1000.00,	// Width
		50);		// Density

	// Determine number of points on the map
	num_map_points = burlington_map.get_density();
	num_map_points *= num_map_points;

	// Dump all the data into the map_mesh
	// Also determine maxes
	float MAX_X, MAX_Y, MAX_Z, MIN_X, MIN_Y, MIN_Z;
	MAX_X = MAX_Y = MAX_Z = FLT_MIN;
	MIN_X = MIN_Y = MIN_Z = FLT_MAX;
	map_mesh = (float*)malloc(sizeof(float) * num_map_points * 3);
	size_t point_index = 0;
	for (size_t i=0; i<num_map_points*3; i+=3) {
		size_t index_x, index_y;

		burlington_map.from_int_to_xy(point_index, &index_x, &index_y);

		//std::cout << index_x << "," << index_y << std::endl;
		std::cout << burlington_map.get_height(index_x, index_y) << std::endl;

		map_mesh[i+0] = index_x * burlington_map.get_spacing_meters();	// X
		map_mesh[i+1] =  burlington_map.get_height(index_x, index_y);	// Y
		map_mesh[i+2] = index_y * burlington_map.get_spacing_meters();	// Z
		map_mesh[i+1] *= 10.0f;

		if (map_mesh[i] > MAX_X) MAX_X = map_mesh[i];
		if (map_mesh[i] < MIN_X) MIN_X = map_mesh[i];
		if (map_mesh[i+1] > MAX_Y) MAX_Y = map_mesh[i+1];
		if (map_mesh[i+1] < MIN_Y) MIN_Y = map_mesh[i+1];
		if (map_mesh[i+2] > MAX_Z) MAX_Z = map_mesh[i+2];
		if (map_mesh[i+2] < MIN_Z) MIN_Z = map_mesh[i+2];

		point_index++;

		std::cout << "(" << map_mesh[i] << "," << map_mesh[i+1] << "," << map_mesh[i+2] << ")" << std::endl;
	}

	// Get Center from maxes
	center_x = MIN_X + (MAX_X - MIN_X) / 2;
	center_y = MIN_Y + (MAX_Y - MIN_Y) / 2;
	center_z = MIN_Z + (MAX_Z - MIN_Z) / 2;

	//glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);

	glMatrixMode(GL_PROJECTION);
	gluPerspective(
		40.0, 	// Field of view (Degrees)
		(1280.0/800.0), 	// aspect ratio
		0.1, 	// z near
		1000000.0);	// z far

	glMatrixMode(GL_MODELVIEW);

	glTranslatef(0.0, 0.0, 1.8);

	// Init 'keyboard'
	for (int i = 0; i < 256; i++)
		keyboard[i] = false;
}

void disp(void) {
	// Clear buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Draw Shit
	glPushMatrix();
	/*
	glutSolidIcosahedron();
	glDisable(GL_LIGHTING);
	glColor4f(1.0, 1.0, 1.0, 1.0);
	glLineWidth(2.0);
	glutWireIcosahedron();
	glEnable(GL_LIGHTING);
	*/
	glPopMatrix();

	glPushMatrix();
	
	float eye_x, eye_y, eye_z;
	eye_x = cos(rotation) * distance;
	eye_z = sin(rotation) * distance;
		 
	gluLookAt(
		eye_x, distance, eye_z,	// eye
		center_x, center_y, center_z, // center
		0.0, 1.0, 0.0);	// up

	glColor3f(1.0f, 1.0f, 1.0f);
	glPointSize(2.0f);
	glBegin(GL_POINTS);
	
	for (int i=0; i<num_map_points*3; i+=3) {
		glVertex3f(map_mesh[i], map_mesh[i+1], map_mesh[i+2]);
	}

	//glVertex3f(0.0f, 0.0f, 0.0f);
	//glVertex3f(0.0f, 1.0f, 0.0f);
	//glVertex3f(2.0f, 0.0f, 0.0f);
	glEnd();
	glPopMatrix();

	// Swap buffers
	glutSwapBuffers();
}

void keyDown(unsigned char key, int x, int y) {
	keyboard[(int)key] = true;
}
void keyUp(unsigned char key, int x, int y) {
	keyboard[(int)key] = false;

	if (key == 'q')
		exit(0);
}

void reshape(int width, int height) {
}

void update() {
	const float ZOOM_SPEED = 100.0f;
	const float ROTATE_SPEED = 0.1f;

	if (keyboard[(int)'w'])
		distance -= ZOOM_SPEED;
	if (keyboard[(int)'s'])
		distance += ZOOM_SPEED;
	if (keyboard[(int)'a'])
		rotation -= ROTATE_SPEED;
	if (keyboard[(int)'d'])
		rotation += ROTATE_SPEED;

	glutPostRedisplay();
}
