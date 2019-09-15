//robot.cpp
//Contains functions for the robots operations and status information also stores the motor array
#include  <pigpiod_if2.h>
#include <stdio.h>
#include <cmath>
#include "robot.hpp"
#include "motorControl.h"
#include "pwmControl.h"
#include "waypoint.hpp"

Robot::Robot(){
	this->x = 0;
	this->y = 0;
	this->xDest = 0;
	this->yDest = 0;
	this->dx = 0;
	this->dy = 0;
}

//Drives to a waypoint, and rotates so we are in the correct orientation at that point
void Robot::Drive(Waypoint waypoint){
	//Set destination
	xDest=waypoint.x;
	yDest=waypoint.y;
	
	//reset our change in x&y
	dx=0;
	dy=0;
	
	//find change in y
	
	dy = yDest-y;
	
	//We will always adjust y first... maybe later we need to deal with avoiding obstacles
	//Unsure
	if(fabs(dy)>DISTANCE_OFFSET*2){//if we aren't within 2x the offset we will allow for.
	//logic for moving in the correct direction based on our orientation
		if(dy>0){//positive y direction
			dy = DISTANCE_CONSTANT;// we will add the dy offset as we move since it is positive
			
			if(direction==NORTH) Move(DIR_FWD);
			else if(direction==EAST) Move(DIR_LEFT);
			else if(direction==SOUTH) Move(DIR_REV);
			else if(direction==WEST) Move(DIR_RIGHT);
		}
		else if(dy<0){//negative y direction
			dy = (-DISTANCE_CONSTANT); //our y-position is decreasing
			
			if(direction==NORTH) Move(DIR_REV);
			else if(direction==EAST) Move(DIR_RIGHT);
			else if(direction==SOUTH) Move(DIR_FWD);
			else if(direction==WEST) Move(DIR_LEFT);
		}
	}	
	dy=0;//reset change in y to be zero;
	
	dx = xDest-x; //find which x direction to travel
	
	if(fabs(dx)>DISTANCE_OFFSET*2){
		if(dx>0){//positive x direction
			dx = DISTANCE_CONSTANT; //our position will update positive x direction
			
			if(direction==NORTH) Move(DIR_RIGHT);
			else if(direction==EAST) Move(DIR_FWD);
			else if(direction==SOUTH) Move(DIR_LEFT);
			else if(direction==WEST) Move(DIR_REV);
		}
		else if(dx<0){//negative x direction
			dx = -DISTANCE_CONSTANT; //position updates negative x direction

			if(direction==NORTH) Move(DIR_LEFT);
			else if(direction==EAST) Move(DIR_REV);
			else if(direction==SOUTH) Move(DIR_RIGHT);
			else if(direction==WEST) Move(DIR_FWD);
		}
	}
	

	if(direction==waypoint.dir); //facing the right way... deal with rotation later
	
	else{
		//do stuff to rotate and face the right way
	}
	//Here we are done with the waypoint. We are facing the Correct orientation.
}

//accepts a defined direction, and sets up motor dir pins to go that way
//might do something with interrupts, so we only enable ones on motors being used

void Robot::Move(int way){
	int temp=0;
	//most movement only uses two motors... we are going to leverage this
	int lowMotor = 0;
	int highMotor = 0;
	
	EncoderCancel();//cancel interrupts
	EncoderInit(membercallback); //start interrupts
	
	//Always kill PWM before moving, just incase it hasn't stopped
	for(int i=0;i<=3;i++){
		changePWM(motor[i].PI_VALUE, HARD_STOP, motor[i].getpwmPIN());
		if (temp != OK) printf("Motor %d failed to stop\n", motor[i].getmotor());
		else printf("Motor %d stopped\n", motor[i].getmotor());
	}
	
	if((way==DIR_ROTATE_CCW)||(way==DIR_ROTATE_CW)){
		//process rotation... maybe put this in the switch statement
	}
	else{//sets up pins based on direction and starts us moving
		//setup gpio pins based on direction we want to drive
		switch(way){
			case DIR_FWD:
				lowMotor=RIGHT_MOTOR;
				highMotor=LEFT_MOTOR;				
				break;
			case DIR_RIGHT:
				lowMotor=REAR_MOTOR;
				highMotor=FRONT_MOTOR;

				break;
			case DIR_REV:
				lowMotor=LEFT_MOTOR;
				highMotor=RIGHT_MOTOR;
	
				break;
			case DIR_LEFT:
				lowMotor=FRONT_MOTOR;
				highMotor=REAR_MOTOR;
				
				break;
			default://handle something here maybe
				break;
			}
		//high and low pins are selected at this point
		gpio_write(motor[lowMotor].PI_VALUE, motor[lowMotor].getdirPIN(), LOW);
		gpio_write(motor[highMotor].PI_VALUE, motor[highMotor].getdirPIN(), HIGH);
		
		motor[lowMotor].changeSpeed(RPM);
		motor[highMotor].changeSpeed(RPM);
		
		motor[lowMotor].percent = INITIAL_SPEED;
		motor[highMotor].percent = INITIAL_SPEED;
		
	//	if(motor[lowMotor].getmotor()==LEFT_MOTOR) motor[lowMotor].percent +=1.0;
	//	if(motor[highMotor].getmotor()==LEFT_MOTOR) motor[highMotor].percent += 1.0;
		
		motor[lowMotor].changeRun(ON);
		motor[highMotor].changeRun(ON);
		
		
		changePWM(PI_VALUE, motor[lowMotor].percent, motor[lowMotor].getpwmPIN());
		changePWM(PI_VALUE, motor[highMotor].percent, motor[highMotor].getpwmPIN());
		//pins are setup, and motors are initialized
		//Robot is now moving in the correct direction
		
		moving = true;
		
		}
	
	while(!stopping);//start spinning until need to stop
	//here we are stopping
	stopping=false;
	
	EncoderCancel();//cancel interrupts
	
	for(int i=0;i<=3;i++){
		
		temp=-1; //set lock and loop until the motor is actually shut off
		//infinitelooop if temp is never OK... DANGER DANGER. DANGER WILL ROBINSON
		while(temp!=OK) temp = changePWM(motor[i].PI_VALUE, HARD_STOP, motor[i].getpwmPIN());
		
		//if (temp != OK) printf("Motor %d failed to stop\n", motor[i].getmotor());
		//else printf("Motor %d stopped\n", motor[i].getmotor());
						
		
		//change the set speeds to be off, and the motors all the be stopped
		motor[i].changeSpeed(OFF);
		motor[i].changeRun(OFF);
		
		moving=false;
				
	}
	//EncoderCancel();//cancel interrupts
	time_sleep(PAUSE); //pause to let everything settle

}


void Robot::setPI(int PI_VALUE){this->PI_VALUE = PI_VALUE;}

void Robot::Setup(re_decoderCB_t callback){
	
	for(int i=0;i<=3;i++){
		motor[i].pinSetup();
	}
	membercallback = callback;
//	EncoderInit(callback);
//	EncoderInit(membercallback);
	
	for(int i=0;i<=3;i++){
		motor[i].display();
		printf("\n");
	}
}

void Robot::EncoderInit(re_decoderCB_t callback){
	for(int i=0;i<=3;i++) motor[i].EncoderSetup(PI_VALUE, motor[i].encA, motor[i].encB, callback);
}

void Robot::EncoderCancel(){
	for(int i=0;i<=3;i++) motor[i].re_cancel();
}


Robot::~Robot(){
	int temp=0;
	//Cancel all callback functions
	EncoderCancel(); //cancels all callbacks
	
	//Shutdown all PWM outputs
	for(int i=0;i<=3;i++){
		temp = changePWM(motor[i].PI_VALUE, HARD_STOP, motor[i].getpwmPIN());
		if (temp != OK) printf("Motor %d failed to stop\n", motor[i].getmotor());
		else printf("Motor %d stopped\n", motor[i].getmotor());
	}

}


