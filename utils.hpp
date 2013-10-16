#ifndef UTILS_H
#define UTILS_H


// Draw a set of coloured cones representing the current local coord system.
// Very handy for debugging transformations!
// X -> red, Y -> green, Z -> blue.
void drawAxis();


// Draw an ellipse with the 3 given radii, centered at the origin.
void drawEllipse(double rx, double ry, double rz);


#endif