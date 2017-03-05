Abstract:
In this project, MATLAB Simulink output an voltage signal to ADuC7026 platform. 
After the A/D signal sampling, ADuC7026 will output two PWM signals to two 
Electronic Speed Controls, which will motivate two motors whose function is 
control the speed of propellers. There are two feedback signals, angle signal 
and angle acceleration signal respectively. The feedback signals will be 
processed by PCI1711, then be transported to MATLAB Simulink.

Files list:
two_propellers.c:		Main function to control ADuC7026. This code 
				needs to be run in Keil uvision together with
				ADuC7026 library.
Demo Vedio.mp4:			A demostration vedio of the two propellers system












