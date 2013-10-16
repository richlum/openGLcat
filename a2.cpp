#include <cstdlib>
#include <cmath>
#include <iostream>
#include <list>
#include <vector>
#include <cstdio>

#ifdef __APPLE__
#  include <GLUT/glut.h>
#elif _WIN32
#  include "GL/glut.h"
#else
#  include <GL/glut.h>
#endif

#include "kitten.hpp"
#include "utils.hpp"


// Global state.
int windowWidth, windowHeight;
int mouseX, mouseY;
double viewTheta = -30;
double viewPhi = 30;
int frameRate = 1;
bool playing = true;
double floorpos = 0;
bool runningx = false;
bool hideAxis = false;

// The kitten!
Kitten *kitten;
// texture stuff
/* Create checkerboard texture */
#define grassImageWidth 1024
#define grassImageHeight 1024
#define roadImageWidth 640
#define roadImageHeight 640
//static GLubyte grassImage[checkImageHeight][checkImageWidth][4];
static GLubyte* grassImage;
static GLubyte* roadImage;
static GLuint tex1Name;
static GLuint tex2Name;
int ctexture =0;



// bitmap loading code from
//http://www.opengl-tutorial.org/beginners-tutorials/tutorial-5-a-textured-cube/


unsigned char* loadfile(const char* fn ){
	
	// Data read from the header of the BMP file
	unsigned char header[54]; // Each BMP file begins by a 54-bytes header
	unsigned int dataPos;     // Position in the file where the actual data begins
	unsigned int width, height;
	unsigned int imageSize;   // = width*height*3
	// Actual RGB data
	unsigned char * data;
	
	
	// Open the file
	FILE * file = fopen(fn,"rb");
	if (!file){
		printf("Image could not be opened\n"); return 0;
	}
	
	
	if ( fread(header, 1, 54, file)!=54 ){ // If not 54 bytes read : problem
		printf("Not a correct BMP file\n");
		return NULL;
	}
	
	if ( header[0]!='B' || header[1]!='M' ){
		printf("Not a correct BMP file\n");
		return 0;
	}
	
	// Read ints from the byte array
	dataPos    = *(int*)&(header[0x0A]);
	imageSize  = *(int*)&(header[0x22]);
	width      = *(int*)&(header[0x12]);
	height     = *(int*)&(header[0x16]);
	
	printf("dataPos = %d , imageSize = %d, width = %d, height = %d\n",
		   dataPos,imageSize,width, height);
	
	// Some BMP files are misformatted, guess missing information
	if (imageSize==0)    imageSize=width*height*3; // 3 : one byte for each Red, Green and Blue component
	if (dataPos==0)      dataPos=54; // The BMP header is done that way
	// Create a buffer
	data = new unsigned char [imageSize];
 
	// Read the actual data from the file into the buffer
	fread(data,1,imageSize,file);
 
	//Everything is in memory now, the file can be closed
	fclose(file);
	return data;

}



// most texture code lifted from redbook example checker.c chapter 9
void initTexture(){
		//makeCheckImage();
	grassImage = loadfile("grass-texture-2.bmp");
	roadImage = loadfile("brick_640x640_rgb.bmp");
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &tex1Name);
	glBindTexture(GL_TEXTURE_2D, tex1Name);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, 
					GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, 
					GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, grassImageWidth, 
					 grassImageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 
					 grassImage);

	//glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &tex2Name);
	glBindTexture(GL_TEXTURE_2D, tex2Name);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, 
					GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, 
					GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, roadImageWidth, 
					 roadImageHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, 
					 roadImage);
}

// One-time configuration of many OpenGL directives.
void initOpenGL() {
    
    // Set clearing values.
    glClearColor(0, 0, 0, 1);
    glClearDepth(1);
    
    // Turn on depth testing.
    glEnable(GL_DEPTH_TEST);
    
    // Turn on blending (for floor).
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Turn on normal normalization (for non-uniform scaling of primitives).
    glEnable(GL_NORMALIZE);
    
    // Turn on lighting.
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
    glEnable(GL_COLOR_MATERIAL);
    
    // Set the intensity of the global ambient light.
    float ambient[] = {0.15, 0.15, 0.3, 1.0};
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
    
    // Set up the diffuse intensities of the directional light.
    float diffuse[] = {0.9, 0.9, 0.8, 1.0}; // yellow for the sun
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);

	GLint maxDepth;
    glGetIntegerv(GL_MAX_MODELVIEW_STACK_DEPTH, &maxDepth);
	std::cout << "GL_MAX_MODELVIEW_STACK_DEPTH: " << maxDepth << std::endl;

	//setup texture
	initTexture();
}




// Called per frame to setup lighting for that frame.
void setupLighting() {
    // w=0 -> direction at infinity.
    float direction[] = {2, 5, 1, 0}; 
	float position[] = {-3, 5, 2, 1}; 
	float spotdirection[] = {-1,-1,-1,1};
	float cutoff[] = {10};
	float ambient[] = {.4,.4,.4,1};
	GLfloat exponent = 128.0;
    glLightfv(GL_LIGHT0, GL_POSITION, direction);
	glLightfv(GL_LIGHT1, GL_POSITION, position);
	glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION,spotdirection);
	glLightfv(GL_LIGHT1, GL_AMBIENT,ambient);
	//glLightfv(GL_LIGHT1, GL_SPOT_CUTOFF,cutoff);
	//glLightfv(GL_LIGHT1, GL_SPOT_EXPONENT,&exponent);
}


// Called by GLUT at the start of the session, and when the window is reshaped.
void reshape(int w, int h) {
    windowWidth = w;
    windowHeight = h;
    glViewport(0, 0, w, h);
}


// Called by GLUT when you move the mouse.
void mouseMove(int x, int y) {
    // GLUT Y coords are inverted.
    y = windowHeight - y;
    
    // Calculate change from last known mouse positon.
    int dx = x - mouseX;
    int dy = y - mouseY;
    
    // Update viewing angles.
    viewTheta = int(viewTheta + 360 + float(dx) / 2) % 360;
    viewPhi = std::min(90.0, std::max(-90.0, viewPhi - dy));
    
    // Remember mouse coords for next time.
    mouseX = x;
    mouseY = y;
}


// Called by GLUT when you click the mouse.
// Simply saving mouse coords.
void mouseClick(int button, int state, int x, int y) {
    // GLUT Y coords are inverted.
    y = windowHeight - y;
    mouseX = x;
    mouseY = y;
}


// Called by GLUT when you press a key on the keyboard.
void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 27 : // escape
	case 'q':
		exit(0);
		break; // You never know... ;)
	case '>':
		frameRate = std::min(128, frameRate * 2);
		break;
	case '<':
		frameRate = std::max(1, frameRate / 2);
		break;
	case 'r':
	case ' ':
		playing = !playing;
		std::cout << "playing = " << playing << std::endl;
		break;
	case 'k':
		kitten->moveToNextPose();
		break;
	case 'j':
		kitten->moveToPrevPose();
		break;
	case '.':
		kitten->incrementPoseIndex(0.1);
		break;
	case ',':
		kitten->incrementPoseIndex(-0.1);
		break;
	case 's':
		kitten->writePoseFile("dump.txt");
		break;
	case 't':
		kitten->cycleTailType();
		break;
	case 'a':
		ctexture = (ctexture+1) % 3;
		break;
	case 'z':
		runningx = !runningx;
		break;
	case 'h':
		hideAxis = !hideAxis;
		break;
	default:
		if (key >= '1' && key <= '9') {
			kitten->setPoseIndex(key - '1');
		} else {
			std::cerr << "unhandled key '" << key << "' (" << int(key) << ")" << std::endl;
		}
		break;
	}
}





// Called by GLUT when we need to redraw the screen.
void display(void) {
    
    // Clear the buffer we will draw into.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Setup camera projection.
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(50.0, double(windowWidth) / double(windowHeight), 0.1, 25);
    
    // Setup camera position/orientation.
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(
        0, 2.5, 10, // eye
        0, 2.5, 0,  // centre
        0, 1  , 0   // up
    );
    glRotated(viewPhi, 1, 0, 0);
    glRotated(viewTheta, 0, 1, 0);
    
    // Must be done after the view is rotated as the lights are stored in
    // eye coords when glLight call is made.
    setupLighting();
    
    // For debugging; show us what the current transformation matrix looks like.
	if (!hideAxis)
        drawAxis();
    
    // Draw the kitten!
    kitten->draw();
    
    // Draw a floor. Since it is transparent, we need to do it AFTER all of
    // the opaque objects.

	if (ctexture!=0) {
		glEnable(GL_TEXTURE_2D);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		if (ctexture == 1){
			glBindTexture(GL_TEXTURE_2D, tex1Name);
		}else{
			glBindTexture(GL_TEXTURE_2D, tex2Name);
		}
		glColor4f(1, 1, 1, 0.75);
		glNormal3f(0, 1, 0);
		glPushMatrix();
		if (runningx)
			glTranslatef(-floorpos, 0, 0);
		glBegin(GL_POLYGON);
		glTexCoord2f(0.0, 0.0);glVertex3f(-5, 0, -5    );
		glTexCoord2f(0.0, 1.0);glVertex3f( 5, 0, -5    );
		glTexCoord2f(1.0, 1.0);glVertex3f( 5, 0,  5    );
		glTexCoord2f(1.0, 0.0);glVertex3f(-5, 0,  5    );
		glEnd();
		glBegin(GL_POLYGON);
		glTexCoord2f(0.0, 0.0);glVertex3f( 5,  0, -5    );
		glTexCoord2f(0.0, 1.0);glVertex3f( 15, 0, -5    );
		glTexCoord2f(1.0, 1.0);glVertex3f( 15, 0,  5    );
		glTexCoord2f(1.0, 0.0);glVertex3f( 5,  0,  5    );
		glEnd();
		glBegin(GL_POLYGON);
		glTexCoord2f(0.0, 0.0);glVertex3f( -15,  0, -5    );
		glTexCoord2f(0.0, 1.0);glVertex3f( -5,    0, -5    );
		glTexCoord2f(1.0, 1.0);glVertex3f( -5,    0,  5    );
		glTexCoord2f(1.0, 0.0);glVertex3f( -15,  0,  5    );
		glEnd();
		glDisable(GL_TEXTURE_2D);
		glPopMatrix();
	}else{
		for (int x = -5; x < 5; x++) {
			for (int y = -5; y < 5; y++) {
				glColor4f(1, 1, 1, (x + y) % 2 ? 0.75 : 0.5);
				glNormal3f(0, 1, 0);
				glBegin(GL_POLYGON);
				glVertex3f(x    , 0, y    );
				glVertex3f(x + 1, 0, y    );
				glVertex3f(x + 1, 0, y + 1);
				glVertex3f(x    , 0, y + 1);
				glEnd();
			}
		}
	}
    // Make the buffer we just drew into visible.
    glutSwapBuffers();
}


// Called by GLUT on a timer for every frame of animation.
// We are responsible for setting the argument that is passed to this function.
void animate(int lastFrame = 0) {
    
    int currentTime = glutGet(GLUT_ELAPSED_TIME);
    int elapsedTime = currentTime - lastFrame;
    
    if (playing) {
        kitten->incrementPoseIndex(frameRate * double(elapsedTime) / 1000);
		//animate the floor in x direction with z key toggling
		floorpos=fmod(floorpos + (frameRate * double(elapsedTime) / 1000),10);
    }
    
    // Signal that the window should be redrawn.
    glutPostRedisplay();
    
    // Schedule the next frame.
    int nextFrame = lastFrame + 1000 / 30;
    glutTimerFunc(std::max(0, nextFrame - currentTime), animate, currentTime);

}


int main(int argc, char *argv[]) {

    // Initialize GLUT and open a window.
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(400, 300);
    glutCreateWindow(argv[0]);
    
    // Register a bunch of callbacks for GLUT events.
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMotionFunc(mouseMove);
    glutMouseFunc(mouseClick);
    glutKeyboardFunc(keyboard);
    
    // Setup OpenGL
    initOpenGL();
    
    // Initialize our kitten.
    kitten = new Kitten();
    if (argc > 1) {
        kitten->readPoseFile(argv[1]);
    }
    
    // Schedule the first animation callback ASAP.
    glutTimerFunc(0, animate, 0);
    
    // Pass control to GLUT.
    glutMainLoop();
    
    // Will never be reached.
    return 0;
}

