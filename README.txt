Compiling and Running on Unix/Linux/...
=============================

The provided Makefile will compile this assignment on a number of Linux/Unix
base systems. All you need to do is run:

    make

To run the executable and pose the kitten in the null pose (all positions and
angles set to 0), simply run it:

    ./a2

You can pass the path to a pose file as the single argument to the executable:

    ./a2 stand.txt

It will automatically begin playing the contained animation at 1 keyframe per
second.


Provided Pose Files
===================

We have included 3 example pose files with this distribution:

- stand.txt: The kitten stands in a relaxed pose.

- beg.txt: A list of keyframes to make the kitten rear up and beg.
  
- debug.txt: A set of poses that sequentially change all coordinates and joint
  angles that are supported in the example solution.


Interface
=========

There are a number of ways to manipulate the running executable. First, you
can drag the view to rotate it. Second, there are a several key bindings:

- q OR esc: exit.

- r OR space: pause/resume animation playback.

- < OR >: decrease or increase the frame rate, respectively.

- j OR k: skip to the previous or next keyframe, respectively; does not stop
  animation playback.
  
- , OR .: move backwards/forwards by 1/10 keyframe, respectively; does not
  stop animation playback.
  
- 1 TO 9: move to specified keyframe; 1, not 0, is the first.

- s: write the current (interpolated) pose of the kitten to a file called
  "dump.txt".

