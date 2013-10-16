#ifndef KITTEN_H
#define KITTEN_H

#include <vector>


// A class to encapsulate a single pose.
struct Pose
{
    Pose(double x, double y, double z, std::vector<double> const &angles)
    :x_(x),y_(y),z_(z),angles_(angles)
    {}

    double x_, y_, z_;
    std::vector<double> angles_;  
};


class Kitten{
private:
    
    // Series of poses that we can take.
    std::vector<Pose> poses_;
    
    // Current index into the poses vector. Fractional values represent
    // interpolation between neighboring poses.
    double poseIndex_; 
    
    // Returns current (interpolated) pose.
    Pose getPose() const;
    
	int specialtailtype;  //0 = 2 section tail, 1 = 10 segments for base and tip, 2 = same as 1 but reverse between base and tip
    
public:
    
    // use initialization list to set default pose index to 0
    Kitten() : poseIndex_(0), specialtailtype(0){}
    
    // Read a series of poses from a file.
    void readPoseFile(char const * path);
    
    // Write the current (interpolated) pose to a file.
    void writePoseFile(char const *path) const;
    
    // Draw the kitten!
    void draw() const;
    
    // Set the poseIndex to the given value.
    void setPoseIndex(double poseIndex);
    
    // Increment the poseIndex by the given value, wrapping around if needed.
    void incrementPoseIndex(double timeChange);
    
    // Set the poseIndex to the previous/next whole keyframe.
    void moveToPrevPose();
    void moveToNextPose();
	
	void cycleTailType(); 
     
};


#endif
