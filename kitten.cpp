#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

#ifdef __APPLE__
#  include <GLUT/glut.h>
#elif _WIN32
#  include "GL/glut.h"
#else
#  include <GL/glut.h>
#endif

#include "utils.hpp"
#include "kitten.hpp"

#define DEBUG 0

// Constants for indexes into the angles std::vector.
//enum JointIndex
//{
//	BODY_ANGLE = 0, NECK_ANGLE, HEAD_ANGLE, FRONT_LEFT_SHOULDER, FRONT_LEFT_KNEE,
//	FRONT_RIGHT_SHOULDER, FRONT_RIGHT_KNEE, BACK_LEFT_SHOULDER, BACK_LEFT_KNEE, 
//	BACK_RIGHT_SHOULDER,  BACK_RIGHT_KNEE, 
//	NUM_ANGLES // This one is not a joint, but a count of how many joints there are.
//};
enum JointIndex
{
	BODY_ANGLE = 0, SHOULDERS, HEAD, FRONT_LEFT_SHOULDER, FRONT_LEFT_KNEE,
	FRONT_RIGHT_SHOULDER, FRONT_RIGHT_KNEE, BACK_LEFT_SHOULDER, BACK_LEFT_KNEE, 
	BACK_RIGHT_SHOULDER,  BACK_RIGHT_KNEE, TAIL_BASE, TAIL_TIP, 
	FR_SHOULDER_X, FL_SHOULDER_X, BR_SHOULDER_X,BL_SHOULDER_X,HEAD_X, HEAD_Y, SHOULDER_X,SHOULDER_Y,
	BODY_ROTX,BODY_ROTY,
	NUM_ANGLES    // This one is not a joint, but a count of how many joints there are.
};

int printed = 0;
GLint lastdepth = -1;

const char* anglearray[]={
    "BODY_ANGLE",
    "SHOULDERS",
    "HEAD",
    "FRONT_LEFT_SHOULDER",
    "FRONT_LEFT_KNEE",
    "FRONT_RIGHT_SHOULDER",
    "FRONT_RIGHT_KNEE",
    "BACK_LEFT_SHOULDER",
    "BACK_LEFT_KNEE",
    "BACK_RIGHT_SHOULDER",
    "BACK_RIGHT_KNEE",
    "TAIL_BASE",
    "TAIL_TIP",
    "FR_SHOULDER_X",
    "FL_SHOULDER_X",
    "BR_SHOULDER_X",
    "BL_SHOULDER_X",
    "HEAD_X",
    "HEAD_Y",
    "SHOULDER_X",
    "SHOULDER_Y",
    "BODY_ROTX",                         
    "BODY_ROTY",
    "NUM_ANGLES" 
};



void printAngles(std::vector<double>* v){
	//for (int i = 0;i< v->size();i++){
	//	std::cout << i << " " << (*v)[i] << std::endl;
	//}
	std::cout <<
		"BODY_ANGLE "          << (*v)[BODY_ANGLE ]         << "\n"
		"SHOULDERS "           << (*v)[SHOULDERS]           << "\n"
		"HEAD "                << (*v)[HEAD     ]           << "\n"
		"FRONT_LEFT_SHOULDER " << (*v)[FRONT_LEFT_SHOULDER] << "\n"
		"FRONT_LEFT_KNEE "     << (*v)[FRONT_LEFT_KNEE    ] << "\n"
		"FRONT_RIGHT_SHOULDER "<< (*v)[FRONT_RIGHT_SHOULDER]<< "\n"
		"FRONT_RIGHT_KNEE "    << (*v)[FRONT_RIGHT_KNEE ]   << "\n"
		"BACK_LEFT_SHOULDER "  << (*v)[BACK_LEFT_SHOULDER]  << "\n"
		"BACK_LEFT_KNEE "      << (*v)[BACK_LEFT_KNEE  ]    << "\n"
		"BACK_RIGHT_SHOULDER " << (*v)[BACK_RIGHT_SHOULDER] << "\n"
		"BACK_RIGHT_KNEE "     << (*v)[BACK_RIGHT_KNEE  ]   << "\n"
		"TAIL_BASE"	 	<< (*v)[TAIL_BASE ]         << "\n"
		"TAIL_TIP"		   << (*v)[TAIL_TIP ]         << "\n"
		;

}

// warning mixed line endings in pose files will screw you up
// windows seems to handle ok but in linux, the  mixed line endings
// will have the affect of appearing to take angles from subsequent
// poses and mashing them into one  pose ... eg multiple seperate lines
// are condensed into one with the most recent angles overwriting earlier angles
void Kitten::readPoseFile(char const *path) {
    
    // Open the file.
    std::ifstream fh;
    fh.open(path);    
    std::vector<double> angles;
    
    while (fh.good()) {
        
        double x, y, z;
        
        angles.clear();
	
        // Read a line from the file.
        std::string line;
        getline(fh, line);
        
        // Skip empty lines, or comments.
        if (!line.size() || line[0] == '#') {
            continue;
        }
        
        // Push the line into a stringstream so we can easily pull out doubles.
        std::stringstream ss;
        ss << line;        
        
        // Read translation coordinates.
        ss >> x;
        ss >> y;
        ss >> z;
        
        // Read angles while there is still stuff to read.
        while (ss.good()) {
            double a;
            ss >> a;
            angles.push_back(a);
        }

	int missing  = NUM_ANGLES- angles.size();
	for (int i = 0; i< missing;i++){
		angles.push_back(0);
	}
	for (int i =0;i<angles.size();i++){
		if (angles[i]!=0){
			std::cout << "pose: " << poses_.size() << " angleno: " << i 
			  << " " << anglearray[i]  <<" angle: " << angles[i] << std::endl;
		}
	}
        
        poses_.push_back(Pose(x, y, z, angles));
    }

}


void Kitten::writePoseFile(char const *path) const {
    
    // Get the current pose.
    Pose pose = getPose();
    
    // Try to open the file.
    std::ofstream outFile;
    outFile.open(path);
    if (!outFile.good()) {
        std::cerr << "could not open '" << path << "' for writing" << std::endl;
        return;
    }
    
    // Write position.
    outFile << pose.x_ << '\t';
    outFile << pose.y_ << '\t';
    outFile << pose.z_ << '\t';
    
    // Write angles.
    for (int i = 0; i < pose.angles_.size(); i++) {
        outFile << pose.angles_[i] << '\t';
    }
    
    // All done!
    outFile.close();
}


Pose Kitten::getPose() const {
    
    double x=0, y=0, z=0;
    std::vector<double> angles(NUM_ANGLES);
    
    // If we don't have any poses then return an empty pose.
    if (!poses_.size()) {
        return Pose(x, y, z, angles);
    }
    
    // If we only have one pose, then return it.
    if (poses_.size() == 1) {
        return poses_[0];
    }
    
    // Return the current pose.
    // NOTE: poseIndex is a double, NOT and int!!! Fractional values represent
    // interpolation between neighboring poses. Eg. 2.5 would be halfway
    // between the 3rd and 4th pose.
	if (poseIndex_ ==floor(poseIndex_)){
		std::cout<< "pose " << poseIndex_ << std::endl;
		return poses_[floor(poseIndex_)];
	}
    
	double proportiondone = (poseIndex_ - floor(poseIndex_))/(ceil(poseIndex_) - floor(poseIndex_)) ;
	//if (proportiondone >= 0.9){
	//	std::cout << "proportion done: " << proportiondone << std::endl;
	//}
	if (ceil(poseIndex_) < poses_.size()){
		//std::cout << "poseIndex:" << poseIndex_ <<" ceil:" << ceil(poseIndex_) << " floor:" << floor(poseIndex_) << std::endl;
		x = proportiondone * (poses_[ceil(poseIndex_)].x_ - poses_[floor(poseIndex_)].x_) + poses_[floor(poseIndex_)].x_ ;
		y = proportiondone * (poses_[ceil(poseIndex_)].y_ - poses_[floor(poseIndex_)].y_) + poses_[floor(poseIndex_)].y_ ;
		z = proportiondone * (poses_[ceil(poseIndex_)].z_ - poses_[floor(poseIndex_)].z_) + poses_[floor(poseIndex_)].z_ ;
		//std::cout << "x,y,z: " << x << ", " << y << ",  " << z << std::endl;
	}else{
		x = (proportiondone) * ( poses_[ceil(0)].x_-poses_[floor(poseIndex_)].x_) + poses_[floor(poseIndex_)].x_ ;
		y = (proportiondone) * ( poses_[ceil(0)].y_-poses_[floor(poseIndex_)].y_) + poses_[floor(poseIndex_)].y_;
		z = (proportiondone) * ( poses_[ceil(0)].z_-poses_[floor(poseIndex_)].z_) + poses_[floor(poseIndex_)].z_ ;
	}
	//std::cout << "proportion:" << proportiondone << " poseIndex_:" << poseIndex_ << std::endl;

	for (int i=0; i<NUM_ANGLES;i++){
		if (ceil(poseIndex_) < poses_.size()){
			if (i<poses_[ceil(poseIndex_)].angles_.size()){
				angles[i]=(proportiondone * (poses_[ceil(poseIndex_)].angles_[i] - poses_[floor(poseIndex_)].angles_[i]))
					+poses_[floor(poseIndex_)].angles_[i];
				//std::cout << "poseIndex:" << poseIndex_ << " cangle:" << poses_[ceil(poseIndex_)].angles_[i] <<
				//	" fangle:" << poses_[floor(poseIndex_)].angles_[i] << " pangle:" << angles.back() << std::endl;
			}else{
				// out of angle data even though there are more poses
				angles[i]=(0);
			}
		}else{
			// this is the last pose
			if (i<poses_[floor(poseIndex_)].angles_.size()){
				//take it back to 0 as proportion done is inverse for last poseIndex
				angles[i]=(proportiondone * (poses_[0].angles_[i]-poses_[floor(poseIndex_)].angles_[i])) 
					+poses_[floor(poseIndex_)].angles_[i];
			}else{
				// no more poses, no more angle data
				angles[i]=(0);
			}
		
		}
	}
	//printAngles(&angles);
	// do we need to force deep copy of angles?
	return (Pose(x,y,z,angles));


    // @@@@ Replace the above return statement with your own code to implement
    // @@@@ linear interpolation.
}


void Kitten::setPoseIndex(double poseIndex) {
    poseIndex_ = std::max(0.0, std::min(double(poses_.size()), poseIndex));
}
    
    
void Kitten::incrementPoseIndex(double timeChange) {
    poseIndex_ = fmod(poseIndex_ + timeChange, poses_.size());
    
    // Need to handle negative deltas.
    if (poseIndex_ < 0) {
        poseIndex_ += poses_.size();
    }
}


void Kitten::moveToPrevPose() {
    if (poses_.size()) {
        poseIndex_ = int(poseIndex_ + poses_.size() - 1) % poses_.size();
    }
}


void Kitten::moveToNextPose() {
    if (poses_.size()) {
        poseIndex_ = int(poseIndex_ + 1) % poses_.size();
    }
}

void Kitten::cycleTailType(){
	specialtailtype = (specialtailtype +1) % 3;
}
void Kitten::draw() const {
    
    // Retrieve the current (interpolated) pose of the kitten.
    Pose pose = getPose();
	// incase we have no pose angles set default to all zero
	if (pose.angles_.size() <NUM_ANGLES){
		for (int i =pose.angles_.size();i<NUM_ANGLES;i++){
			pose.angles_.push_back(0.0);
		}
	}
	if (DEBUG){
		printed++;
		std::cout << pose.x_ << ", " << pose.y_ << ", " << pose.z_ << "  x,y,z" << std::endl;
		int i =0;
		//for (std::vector<double>::iterator it = pose.angles_.begin(); it != pose.angles_.end();it++){
		////for (int i = 0; i < pose.angles_.size(); i++) {
		//	std::cout << "\t" << i++ << "\t" << *it << '\n';
		//}
		printAngles(&pose.angles_);
	}
	

    // The kitten should be dark brown.
    glColor4d(0.46f, 0.3f, 0.13f, 1);
    
    // @@@@ Add your own code to draw the kitten. You might want to split
    // @@@@ this into multiple functions for drawing different parts of the
    // @@@@ creature.
	
	GLfloat overlap = .2; // factor to adjust object seperation down by
	// midsection
	glPushMatrix();  // global matrix save
	glTranslatef(pose.x_,pose.y_,pose.z_);
	GLfloat midXRadius = .8;
	GLfloat midYRadius = .8;
	GLfloat midZRadius = .6;
	GLfloat midx = 0;
	GLfloat midy = 0;
	GLfloat midz = 0;
	glRotatef(pose.angles_[BODY_ANGLE],0,0,1);
	glRotatef(pose.angles_[BODY_ROTX],1,0,0);
	glRotatef(pose.angles_[BODY_ROTY],0,1,0);
	drawEllipse(midXRadius,midYRadius,midZRadius);

	// chest
	GLfloat ChestXRadius = .8;
	GLfloat ChestYRadius = 1.0;
	GLfloat ChestZRadius = .7;
	GLfloat Chestx = ChestXRadius + midXRadius;
	GLfloat Chesty = midy;
	GLfloat Chestz = midz;
	glTranslatef(midXRadius,0,0);
	glRotatef(pose.angles_[SHOULDERS], 0,0,1);
	glRotatef(pose.angles_[SHOULDER_X], 1,0,0);
	glRotatef(pose.angles_[SHOULDER_Y], 0,1,0);
	//glTranslatef(Chestx*(1-overlap),Chesty,Chestz);
	glTranslatef(ChestXRadius*(1-overlap),Chesty,Chestz);
	
	drawEllipse(ChestXRadius,ChestYRadius,ChestZRadius);
	glPushMatrix();// save Chest center for legs after done with head
	
	// head
	GLfloat headXRadius = .6;
	GLfloat headyRadius = .4;
	GLfloat headzRadius = .6;
	GLfloat headWRTchestAngle=45;
	//GLfloat headx = ChestXRadius*(cos(headWRTchestAngle))+headXRadius;
	//GLfloat heady = ChestXRadius*sin(headWRTchestAngle)+headyRadius;
	//GLfloat headz = Chestz;
	//GLfloat headRotationCntrx = ChestXRadius*(cos(headWRTchestAngle)*cos(headWRTchestAngle))*(1-overlap);//wrt chest
	//GLfloat headRotationCntry = ChestYRadius*(sin(headWRTchestAngle)*sin(headWRTchestAngle))*(1-overlap);
	GLfloat headRotationCntrx = ChestXRadius*ChestXRadius*(cos(headWRTchestAngle)*cos(headWRTchestAngle));//wrt chest
	GLfloat headRotationCntry = ChestYRadius*ChestYRadius*(sin(headWRTchestAngle)*sin(headWRTchestAngle));
	GLfloat headRotationCntrz = 0;

	glTranslatef(headRotationCntrx,headRotationCntry,headRotationCntrz);
	glRotatef(pose.angles_[HEAD], 0,0,1);
	glRotatef(pose.angles_[HEAD_X], 1,0,0);
	glRotatef(pose.angles_[HEAD_Y], 0,1,0);
	glTranslatef(headXRadius, headyRadius,midz);
	glPushMatrix();  // save head center for second ear
	drawEllipse(headXRadius,headyRadius,headzRadius);

	// right ear
	glTranslatef(-0.2*headXRadius, headyRadius*(1-overlap), headzRadius*.8); 
	drawEllipse(.1,.25,.2);
	
	// left ear
	glPopMatrix(); // restore head center
	glTranslatef(-0.2*headXRadius, headyRadius*(1-overlap), -headzRadius*.8); 
	drawEllipse(.1,.25,.2);
	//glPopMatrix();

	// front legs
	glPopMatrix();   // restore chest center
	glPushMatrix();  // save chest center for second leg
	GLfloat thighxradius = .3; 
	GLfloat thighyradius = .5;
	GLfloat thighzradius = .4;
	//right front thigh
	GLfloat rfThighxPos = 0; // wrt chest center
	GLfloat rfThighyPos = -ChestYRadius*.3;
	GLfloat rfThighzPos = ChestZRadius*1.0;
	glTranslatef(rfThighxPos,rfThighyPos,rfThighzPos);
	glRotatef(pose.angles_[FRONT_RIGHT_SHOULDER],0,0,1);
	glRotatef(pose.angles_[FR_SHOULDER_X],1,0,0);
	glTranslatef(0,-thighyradius,0);
	drawEllipse(thighxradius,thighyradius,thighzradius);

	//right front shin
	GLfloat shinxradius = .2; 
	GLfloat shinyradius = .6;
	GLfloat shinzradius = .2;
	glTranslatef(0,-thighyradius,0);
	glRotatef(pose.angles_[FRONT_RIGHT_KNEE], 0,0,1);
	glTranslatef(0,-shinyradius*(1-overlap),0);
	drawEllipse(shinxradius,shinyradius,shinzradius);
	
	//left front thigh
	glPopMatrix(); // recover chest center for second leg
	GLfloat lfThighxPos = 0; // wrt chest center
	GLfloat lfThighyPos = -ChestYRadius*.3;
	GLfloat lfThighzPos = -ChestZRadius*1.0;
	glTranslatef(lfThighxPos,lfThighyPos,lfThighzPos);
	glRotatef(pose.angles_[FRONT_LEFT_SHOULDER],0,0,1);
	glRotatef(pose.angles_[FL_SHOULDER_X],1,0,0);
	glTranslatef(0,-thighyradius,0);
	drawEllipse(thighxradius,thighyradius,thighzradius);

	//left front shin
	glTranslatef(0,-thighyradius,0);
	glRotatef(pose.angles_[FRONT_LEFT_KNEE], 0,0,1);
	glTranslatef(0,-shinyradius*(1-overlap),0);
	drawEllipse(shinxradius,shinyradius,shinzradius);

	glPopMatrix(); // restore global
	glPushMatrix(); // save global 
	// back legs
	// back right thigh
	
	glTranslatef(pose.x_,pose.y_,pose.z_);
	glRotatef(pose.angles_[BODY_ANGLE],0,0,1);
	glRotatef(pose.angles_[BODY_ROTX],1,0,0);
	glRotatef(pose.angles_[BODY_ROTY],0,1,0);
	GLfloat backThighXRadius = .5;
	GLfloat backThighYRadius = .6;
	GLfloat backThighZRadius = .3;
	glTranslatef(-midXRadius/2,-midYRadius/3*(1-overlap),midZRadius*(1-overlap));
	glRotatef(pose.angles_[BACK_RIGHT_SHOULDER],0,0,1);
	glRotatef(pose.angles_[BR_SHOULDER_X],1,0,0);
	glTranslatef(0,-backThighYRadius,0);
	drawEllipse(backThighXRadius,backThighYRadius,backThighZRadius);

	// back right shin
	GLfloat backShinXRadius = .2;
	GLfloat backShinYRadius = .5;
	GLfloat backShinZRadius = .2;
	glTranslatef(0,-backThighYRadius,0);
	glRotatef(pose.angles_[BACK_RIGHT_KNEE],0,0,1);
	glTranslatef(0,-backShinYRadius*(1-overlap),0);
	drawEllipse(backShinXRadius,backShinYRadius,backShinZRadius);
	glPopMatrix(); // restore global
	glPushMatrix(); // save global
	
	// back left thigh
	glTranslatef(pose.x_,pose.y_,pose.z_);
	glRotatef(pose.angles_[BODY_ANGLE],0,0,1);
	glRotatef(pose.angles_[BODY_ROTX],1,0,0);
	glRotatef(pose.angles_[BODY_ROTY],0,1,0);
	glTranslatef(-midXRadius/2,-midYRadius/2*(1-overlap),-midZRadius*(1-overlap));
	glRotatef(pose.angles_[BACK_LEFT_SHOULDER],0,0,1);
	glRotatef(pose.angles_[BL_SHOULDER_X],1,0,0);
	glTranslatef(0,-backThighYRadius,0);
	drawEllipse(backThighXRadius,backThighYRadius,backThighZRadius);

	// back left shin
	glTranslatef(0,-backThighYRadius,0);
	glRotatef(pose.angles_[BACK_LEFT_KNEE],0,0,1);
	glTranslatef(0,-backShinYRadius*(1-overlap),0);
	drawEllipse(backShinXRadius,backShinYRadius,backShinZRadius);

	glPopMatrix();// restore global
	glPushMatrix();// save global state
	
	
	GLfloat tailBaseXRadius = .40;
	GLfloat tailBaseYRadius = 0.15;
	GLfloat tailBaseZRadius = 0.15;
	GLfloat tailTipXRadius = .40;
	GLfloat tailTipYRadius = 0.15;
	GLfloat tailTipZRadius = 0.15;
	glTranslatef(pose.x_,pose.y_,pose.z_);
	glRotatef(pose.angles_[BODY_ANGLE],0,0,1);
	glRotatef(pose.angles_[BODY_ROTX],1,0,0);
	glRotatef(pose.angles_[BODY_ROTY],0,1,0);
	glTranslatef(-midXRadius,0,0);
	
	if (specialtailtype==0){
		glRotatef(pose.angles_[TAIL_BASE],0,0,1);
		glTranslatef(-tailBaseXRadius*(1-overlap),0,0);
		drawEllipse(tailBaseXRadius,tailBaseYRadius,tailBaseZRadius);

		glTranslatef(-tailBaseXRadius,0,0);
		glRotatef(pose.angles_[TAIL_TIP],0,0,1);
		glTranslatef(-tailTipXRadius*(1-overlap),0,0);
		drawEllipse(tailTipXRadius,tailTipYRadius,tailTipZRadius);
	}else if (specialtailtype >=1){
		int segments = 20;
		double segmentradius = 2*tailBaseXRadius/segments;
		for (int i = 0; i< segments;i++){
			glRotatef(pose.angles_[TAIL_BASE]/segments,0,0,1);
			glTranslatef(-segmentradius*(1-overlap),-0,0);
			drawEllipse(tailBaseYRadius,tailBaseYRadius,tailBaseZRadius);
		}
		int tipreverse;
		if (specialtailtype==1)
			tipreverse=1;
		else
			tipreverse=-1;
		segmentradius = 2*tailTipXRadius/segments;
		glTranslatef(-segmentradius*(1-overlap),0,0);
		for (int i = 0; i< segments;i++){
			glRotatef(pose.angles_[TAIL_TIP]/segments*tipreverse,0,0,1);
			glTranslatef(-segmentradius*(1-overlap),0,0);
			drawEllipse(tailBaseYRadius,tailTipYRadius,tailTipZRadius);
		}

	}
	glPopMatrix(); // restore global state

	GLint depth;	
	glGetIntegerv(GL_MODELVIEW_STACK_DEPTH, &depth);
	// this should never exceed max (32 typical) and should
	// really be set back to one by the time we reach bottom of loop
	if (depth > lastdepth){
		std::cout << "matrix depth: " << depth << std::endl;
		lastdepth = depth;
	}




}

