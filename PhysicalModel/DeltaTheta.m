% This checks deltaD by calculating deltaTheta
% to see if the equation holds
% The result of deltaTheta should be 
% deltaTheta*distToPocket=PocketRadius
clear all

% Phi (convert to radians)
phi=0*pi/180; 

% Change in phi (converted to radians)
deltaPhi=1.3636*pi/180;

% Distance to cue ball (feet converted to metres)
d=2.5*0.3048;

% Radius of balls (metres)
R=0.028575;

% x is used to make the deltaTheta equation simpler
x=(d/(2*R))*sin(deltaPhi);

% Calculates deltaTheta
deltaTheta=(phi-deltaPhi+asin(x-sin(phi-deltaPhi)));

% Prints deltaTheta in degrees
inDegrees=deltaTheta*180/pi

% distance to pocket (metres)
distToPocket=sqrt(2)*0.3048;

% Hit radius for delta phi given certain distance to pocket
hitRadius=distToPocket*sin(deltaTheta)
