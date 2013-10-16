#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <sstream>

#ifdef __APPLE__
#  include <GLUT/glut.h>
#elif _WIN32
#  include "GL/glut.h"
#else
#  include <GL/glut.h>
#endif

#include "utils.hpp"


void drawAxis() {
	//glutSolid cone draws along z axis, so rotate to desired axis then draw.
    // Draw X axis in red
    glPushMatrix();
    glRotatef(90, 0, 1, 0);
    glColor3f(1, 0, 0);
    glutSolidCone(0.1, 1, 8, 2);
    glPopMatrix();
    
    // Draw Y axis in green.
    glPushMatrix();
    glRotatef(-90, 1, 0, 0);
    glColor3f(0, 1, 0);
    glutSolidCone(0.1, 1, 8, 2);
    glPopMatrix();
    
    // Draw Z axis in blue
    glColor3f(0, 0, 1);
    glutSolidCone(0.1, 1, 8, 2);
}


void drawEllipse(double rx, double ry, double rz) {
    // @@@@ Add your own code to draw an ellipse with the 3 given radii.
	glPushMatrix();
	//x^2/a^2 + y^2/b^2 + z^2/c^2 = 1 ellipse. so x = a when y and z = 0.
	//so we want to scale to respective a b c (squared)
	//glScalef((GLfloat)1.0*sqrt(rx), (GLfloat)1.0*sqrt(ry), (GLfloat)1.0*sqrt(rz));
	glScalef((GLfloat)(rx), (GLfloat)(ry), (GLfloat)(rz));
	glutSolidSphere(1,20,20);
	glPopMatrix();
	GLenum error = glGetError();
	if (error != GL_NO_ERROR)
		std::cout << "GL ERROR:" << error << std::endl;

}


