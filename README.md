BilliardBuddy
=============

A SYDE 461 design project for a mediated-reality training system for sports (e.g. billiards).

BilliardBuddy runs on a custom-made mediated-reality headset composed of an Oculus DK2 and 
a stereoscopic vision system constructed from two Logitech C310 webcams modified with
wide-angle (120° FOV) lenses, inspired by William Steptoe's AR-Rift project 
(http://willsteptoe.com/post/66968953089/ar-rift-part-1).

BilliardBuddy provides augmented training cues (no pun intended!) to beginner billiards
players, designed to be a more portable alternative to the AR-Pool project (http://arpool.ca/).


Installation
============

BilliardBuddy depends on OpenCV, OpenCL, and Oculus SDK. We currently only support 64-bit Windows.

1. Install OpenCV 2.4.9 according to: http://docs.opencv.org/doc/tutorials/introduction/windows_install/windows_install.html, being sure to properly set the OPENCV_DIR environment variable and adding the "opencv\build\x64\vc12\bin" path to the PATH envirnmonment variable.

2. Install OpenCL drivers for your OpenCL device (e.g. AMD, Nvidia, or Intel).

3. Install Oculus SDK 0.4.4 and set the OCULUS_SDK_DIR to the installation directory.

4. Clone the BilliardBuddy project into your development evironment such as Visual Studio 2013.