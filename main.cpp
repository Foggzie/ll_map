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
ll_map my_map;
void build_map(int argc, char** argv);

// Functions for GL delegates
void disp();
void init();
void keyUp(unsigned char, int, int);
void keyDown(unsigned char, int, int);
void reshape(int, int);
void update();

// Window ID
static int win;

// Light
GLfloat light0_diffuse[] = {1.0, 1.0, 1.0, 1.0};
GLfloat light0_position[] = {-1.0, 0.0, 0.0, 0.0};

// For key input
static bool keyboard[256];

// Camera Stuff
static float move_speed = 1.0f;
static float distance = 5.0f;
static float height = 1.0f;
static float rotation = 0.0f;
static float center_x;
static float center_y;
static float center_z;

// All the 'll_map' dumped into a mesh
static float MAX_X, MAX_Y, MAX_Z, MIN_X, MIN_Y, MIN_Z;
static float* map_mesh;
static float* map_strip;
static size_t num_map_points;
 
int main(int argc, char** argv) {
	
	build_map(argc, argv);

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

	glClearColor(0.00, 0.00, 0.00, 1.0);

	init();

	glutMainLoop();

	delete[] map_mesh;
	//delete[] map_strip;
	return 0;
}

void build_map(int argc, char** argv) {
	if (argc != 5)
		// Read the elevation data from MapQuest
		my_map.build_map(
			44.476221,	// Latitude
			-73.205595,	// Longitude
			4000.00,	// Width
			50);		// Density

	else
		// Read the elevation data from MapQuest
		my_map.build_map(
			atof(argv[1]),
			atof(argv[2]),
			atof(argv[3]),
			atof(argv[4]));

	// Determine number of points on the map
	num_map_points = my_map.get_density();
	num_map_points *= num_map_points;

	// Dump all the data into the map_mesh
	// Also determine maxes
	MAX_X = MAX_Y = MAX_Z = FLT_MIN;
	MIN_X = MIN_Y = MIN_Z = FLT_MAX;
	map_mesh = (float*)malloc(sizeof(float) * num_map_points * 3);
	size_t point_index = 0;
	for (size_t i=0; i<num_map_points*3; i+=3) {
		size_t index_x, index_y;

		my_map.from_int_to_xy(point_index, &index_x, &index_y);

		//std::cout << index_x << "," << index_y << std::endl;
		std::cout << my_map.get_height(index_x, index_y) << std::endl;

		map_mesh[i+0] = index_x * my_map.get_spacing_meters();	// X
		map_mesh[i+1] =  my_map.get_height(index_x, index_y);	// Y
		map_mesh[i+2] = index_y * my_map.get_spacing_meters();	// Z
		map_mesh[i+1] *= 1; //my_map.get_spacing_meters(); // 10.0f;

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

	move_speed = my_map.get_width_meters() / 1000.0f;
}

void init() {
	glEnable(GL_DEPTH_TEST);

	glMatrixMode(GL_PROJECTION);
	gluPerspective(
		40.0, 	// Field of view (Degrees)
		(1280.0/800.0), 	// aspect ratio
		0.1, 	// z near
		1000000.0);	// z far

	glMatrixMode(GL_MODELVIEW);

	glTranslatef(0.0, 0.0, 1.8);

	// Setup a diffuse light
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, light0_position);

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
	eye_x = center_x + cos(rotation) * distance;
	eye_z = center_z + sin(rotation) * distance;
		 
	gluLookAt(
		eye_x, center_y + height, eye_z,	// eye
		center_x, center_y, center_z, // center
		0.0, 1.0, 0.0);	// up

	glColor3f(1.0f, 1.0f, 1.0f);
	glPointSize(2.0f);
	
	const float min_color = 0.01f;
	const float max_color = 1.0f;
	float y_range = MAX_Y - MIN_Y;
	float color_range = max_color - min_color;
	glBegin(GL_POINTS);
	for (int i=0; i<num_map_points*3; i+=3) {
		float y_percent = (map_mesh[i+1] - MIN_Y) / y_range;
		float color = min_color + y_percent * color_range;
		glColor3f(color, color, color);
		glVertex3f(map_mesh[i], map_mesh[i+1], map_mesh[i+2]);
	}
	glEnd();

	/*
	// Draw triangle strips across
	size_t index_x, index_y, index_linear;
	
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glBegin(GL_TRIANGLE_STRIP);
	for (int j=0; j<my_map.get_density()-1; j++) {
	for (int i=0; i<my_map.get_density(); i++) {
		index_x = i;
		index_y = j;

		index_linear = my_map.get_density() * index_y + index_x;
		index_linear *= 3;
		glVertex3f(
			map_mesh[index_linear],
			map_mesh[index_linear+1],
			map_mesh[index_linear+2]);

		index_y = j+1;

		index_linear = my_map.get_density() * index_y + index_x;
		index_linear *= 3;
		glVertex3f(
			map_mesh[index_linear],
			map_mesh[index_linear+1],
			map_mesh[index_linear+2]);
	}
	}
	glEnd();
	glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT0);
	*/
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
	const float ROTATE_SPEED = 0.05f;

	if (keyboard[(int)'w'])
		distance -= move_speed;
	if (keyboard[(int)'s'])
		distance += move_speed;
	if (keyboard[(int)'f'])
		height -= move_speed;
	if (keyboard[(int)'r'])
		height += move_speed;
	if (keyboard[(int)'a'])
		rotation -= ROTATE_SPEED;
	if (keyboard[(int)'d'])
		rotation += ROTATE_SPEED;

	glutPostRedisplay();
}
