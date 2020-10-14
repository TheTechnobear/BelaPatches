# Calibration

Belas input and outputs are **not** calibrated.

therefore an output of 0.1, might theoretically mean 1v, but in practice it might put out 0.9975 volts

if we wish to read or write accurate voltages we need to calibrate them by determining scaling and offset values.

when are accurate voltages important?
a good example of accurate voltages requirements are eurorack pitch inputs - these are volt/octave.

so if 1v = C3, if we output 1.03v , this is actually off by 0.03/12 semitones, or 360 cents, which is quite noticable.
worst still, often we will find without calibration the 'scaling' is wrong, so as we put out more voltage it becomes increasling out of scale.


This patch aims to produce this calibration data



# Methodolgy  

the methodolgy is very simple, we are using linear scaling

e.g. 

float/expected/actual
0.0 / 0v / 0.02
0.1 / 1v / 1.12
0.2 / 2v / 2.22

here we can see there is a 0.02 offset , and also a scaling of 0.1 that needs correction
the relationship may however not be linear,however to keep things computationally 'inexpensive'
this calibration model will only use 11 points (0.1 intervals) and linearly interpret between them.



# compiling using xcBela

xcCompileRun calibration


# using calibration data

this patch outputs files which can be incorporated into C++ or Supercollider patches to easily calibrate outputs.
currently this is not 'invisible' rather it is delibaretely done by the patch.
this is because not ALL I/O needs to be calibrate, and it has a 'performance cost' , so it up to the patch to choose when and when  I/O are calibrated.
