% Calculates the allowable angle off from 
% center to still sink a pool ball
% Note: Allowable Angle Diagram shows location of all variables
clear all

% Radius of pocket (metres) used 5 inches as pocket diameter
R=0.0635;

% Length from pocket to solid ball (metres)
L=sqrt(2*(0.5^2))*0.3048; %conversion feet to meters 0.3048

% radius of ball (metres)
r=0.028575;

% distance from cue ball to solid ball (metres)
l=2.5*0.3048; %converted from feet to metres

% Angle between ball and pocket relative to wall (radians)
a=45*pi/180;

% Angle from target ball between middle of pocket and side of pocket
b=asin(R/L);

% Angle from cue ball between solid ball and centered ghost ball
% x and y are placeholders to simplify expression
x=2*r*sin(a);
y=sqrt((4*r^2)+(l^2)-(4*r*l*cos(a)));
c=asin(x/y);

% Allowable angle
% u and v are placeholders
u=2*r*sin(a+b);
v=sqrt((4*r^2)+(l^2)-(4*r*l*cos(a+b)));
d=asin(u/v)-c;

angle=d*180/pi

