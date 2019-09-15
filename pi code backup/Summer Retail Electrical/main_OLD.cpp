//main.c
//sudo pigpiod -s 10 *Sets up daemon if stopped to the right sampling rate
#include <stdio.h>
#include <cmath>
#include <pigpiod_if2.h>
#include <wiringPi.h>
#include "init.h"
#include "pwmControl.h"
#include "motorControl.h"
#include "piCleanup.h"
#include "robot.hpp"

#define LOW 0
#define HIGH 1

#define OK 0

//direction defines for Start_Drive function
#define DIR_FWD 0
#define DIR_REV 1
#define DIR_RIGHT 2
#define DIR_LEFT 3
#define DIR_ROTATE_CCW 4
#define DIR_ROTATE_CW 5
#define DIR_STOP -1
#define DIR_EXIT -2


#define INITIAL_SPEED 20

static Robot robot; //robot global object


//callback function Only gets called for Front/Right Motor
//Will use for navigation. Ignore if running is false

void callback(int motor, int direction){
	
	static bool locked = false;
	
	//only process for running motors... Might add some compensation later for not running interrupts
	if(robot.running){
		while(locked); //spin-lock to avoid simultaneous access
		locked = true;//lock function
			
		if(motor==RIGHT_MOTOR){
			robot.y += direction*DISTANCE_CONSTANT;
			if (fabs(robot.y-robot.yDest)<=3) robot.stopping = true; //Might change, but basically if the difference is <= 3mm. Stop
			
			robot.done=true; //THIS WILL BE SET ON LAST WAYPOINT LATER
		}
		
		
		/*
		This block finds if our x or y position should update. This is based on the direction we are driving.
		This may be triggered on a motor not spinning... we will only process cases that should exist.
				
		y and x position tracking and next destination values
		distance travelled is derived from tick count on tire totation
		Circumference of 319.19mm/90 ticks in X1 mode = 3.547mm/tick
		Stored in DISTANCE_CONSTANT define
			
		Float position of Robot. Processed in mm. Is measured as distance from 'home'
		North[Front at start] (y)+, East[Right at start] (x)+
		
		FWD=+1
		REV=-1
		* 
		*/
		//Direction updates WILL need adjusting/confirmation ***FUTURE IMPLEMENTATION***
		
		/*
		if (robot.direction==NORTH){
			if(motor==RIGHT_MOTOR) robot.y += direction*PULSE_CONSTANT; 	 //FWD - y+; REV - y-
			else if (motor==FRONT_MOTOR) robot.x -=direction*PULSE_CONSTANT; //FWD - x-; REV - x+
		}
		else if (robot.direction==SOUTH){
			if(motor==RIGHT_MOTOR) robot.y -= direction*PULSE_CONSTANT; 	 //FWD - y-; REV - y+
			else if (motor==FRONT_MOTOR) robot.x +=direction*PULSE_CONSTANT; //FWD - x+; REV - x-
		}
		
		else if (robot.direction==EAST){
			if(motor==RIGHT_MOTOR) robot.x += direction*PULSE_CONSTANT; 	 //FWD - y+; REV - y-
			else if (motor==FRONT_MOTOR) robot.y -=direction*PULSE_CONSTANT; //FWD - x-; REV - x+
		}
		else if (robot.direction==WEST){
			if(motor==RIGHT_MOTOR) robot.x += direction*PULSE_CONSTANT; 	 //FWD - y+; REV - y-
			else if (motor==FRONT_MOTOR) robot.y -=direction*PULSE_CONSTANT; //FWD - x-; REV - x+
		}
		* */

		robot.count++;
		//if(robot.count>3000) robot.running=false;
		
		printf("count: %d\n", robot.count);
		printf("Y-position: %0.2f\n", robot.y);
		
		locked = false; //unlock function
	}
}


int main(){
	
	float RPM = 20;//Desired running RPM
	
	int PI_VALUE = 0;
	int temp = 0;

	PI_VALUE = init();
	
	if (PI_VALUE < 0){
		printf("Something didn't start check if daemon running\n");
		return PI_VALUE;
	}
	
	//Use this to verify PWM output on PIN 19 as a test of hardware
	/*
	temp = set_mode(PI_VALUE, 19, PI_OUTPUT);
	temp = gpio_write(PI_VALUE,19,HIGH);
	temp = set_PWM_dutycycle(PI_VALUE, 19, 127);
	*getchar();
	*/
	
	//Create object array for motors (loc,PI,dirPIN, pwmPIN) as defined in motorControl.h
	//might overload Constructor later for cleaner initialization. THIS NEEDS TO GO INTO ROBOT CLASS
	/*
	Motor motor[4] = {
		Motor(FRONT_MOTOR, PI_VALUE, FRONT_MOTOR_DIR, FRONT_MOTOR_PWM, FRONT_MOTOR_ENC_A, FRONT_MOTOR_ENC_B),
		Motor(RIGHT_MOTOR, PI_VALUE, RIGHT_MOTOR_DIR, RIGHT_MOTOR_PWM, RIGHT_MOTOR_ENC_A, RIGHT_MOTOR_ENC_B),
		Motor(REAR_MOTOR, PI_VALUE, REAR_MOTOR_DIR, REAR_MOTOR_PWM, REAR_MOTOR_ENC_A, REAR_MOTOR_ENC_B),
		Motor(LEFT_MOTOR, PI_VALUE, LEFT_MOTOR_DIR, LEFT_MOTOR_PWM, LEFT_MOTOR_ENC_A, LEFT_MOTOR_ENC_B)
	};
	*/
	//Call Setup Function of each Motor. Done outside Constructor for future exceptions
	//Move to an init function later for cleaner code.
	//also sets up the encoder for each motor
	for(int i=0;i<=3;i++){
		motor[i].pinSetup();
		motor[i].EncoderSetup(PI_VALUE, motor[i].encA, motor[i].encB, callback);
	}
	
	

	
	getchar();//Just to pause
	/*
	printf("Enter a X-Distance in mm to travel");
	scanf("%f",&robot.xDest);
	printf("Enter a Y-Distance in mm to travel");
	scanf("%f",&robot.yDest);
	* */
	robot.yDest = 203;
	robot.running = true; //Start the robot in 'run mode' To be used later so it doens't run before a command
	
for(int times=0;times<=9;times++){
	
	while(robot.running){

		//If we aren't currently moving, first make sure all motor PWM has stopped incase of bad start state
		if (!robot.moving){
			
			for(int i=0;i<=3;i++){
				temp = changePWM(motor[i].PI_VALUE, OFF, motor[i].getpwmPIN());
				if (temp != OK) printf("Motor %d failed to stop\n", motor[i].getmotor());
				else printf("Motor %d stopped\n", motor[i].getmotor());
			}
				
			//Move in the positive y direction
			//Drives Forward(y+ direction) Later we need to use a function based on a direction
			//Will need to add in an offset so we are ok with being a few mm out of alignment
			if(robot.yDest > robot.y){ //deals with moving in positive y direction
			
				motor[RIGHT_MOTOR].changeDir(FWD);
				
				gpio_write(motor[RIGHT_MOTOR].PI_VALUE, motor[RIGHT_MOTOR].getdirPIN(), LOW);
				gpio_write(motor[LEFT_MOTOR].PI_VALUE, motor[LEFT_MOTOR].getdirPIN(), HIGH);
				
				changePWM(PI_VALUE, INITIAL_SPEED, motor[RIGHT_MOTOR].getpwmPIN());
				changePWM(PI_VALUE, INITIAL_SPEED, motor[LEFT_MOTOR].getpwmPIN());
	
				motor[RIGHT_MOTOR].percent = INITIAL_SPEED;
				motor[LEFT_MOTOR].percent = INITIAL_SPEED;
				
				motor[RIGHT_MOTOR].changeSpeed(RPM);
				motor[LEFT_MOTOR].changeSpeed(RPM);
				
				robot.moving = true;
			
			}
			
			else if(robot.yDest < robot.y){ //deals with moving in negative y direction
				
				motor[RIGHT_MOTOR].changeDir(REV);
				
				gpio_write(motor[RIGHT_MOTOR].PI_VALUE, motor[RIGHT_MOTOR].getdirPIN(), HIGH);
				gpio_write(motor[LEFT_MOTOR].PI_VALUE, motor[LEFT_MOTOR].getdirPIN(), LOW);
				
				changePWM(PI_VALUE, INITIAL_SPEED, motor[RIGHT_MOTOR].getpwmPIN());
				changePWM(PI_VALUE, INITIAL_SPEED, motor[LEFT_MOTOR].getpwmPIN());
	
				motor[RIGHT_MOTOR].percent = INITIAL_SPEED;
				motor[LEFT_MOTOR].percent = INITIAL_SPEED;
				
				motor[RIGHT_MOTOR].changeSpeed(RPM);
				motor[LEFT_MOTOR].changeSpeed(RPM);
				
				robot.moving = true;
			}
			
			//Here handle if we are good for y-direction, but need to change x direction
		}
		//If we are stopping based on positional data, probably do comparison of tick count here, and adjust true X/Y positional offsets
		else if(robot.stopping){
	
			robot.stopping = false;//we have entered the stopping routine... fix later to better deal with it.
			//stop the motors
			for(int i=0;i<=3;i++){
				motor[i].re_cancel();//when stopping, cancel all callbacks to avoid error in RPM/Speed calculations
				
				temp = changePWM(motor[i].PI_VALUE, OFF, motor[i].getpwmPIN());
				if (temp != OK) printf("Motor %d failed to stop\n", motor[i].getmotor());
				else printf("Motor %d stopped\n", motor[i].getmotor());
								
				
				//change the set speeds to be off.
				motor[i].changeSpeed(OFF);
				
			}
			time_sleep(1);
			
			for (int i=0;i<=3;i++) motor[i].EncoderSetup(PI_VALUE, motor[i].encA, motor[i].encB, callback);	
			robot.moving = false;
			
			if(robot.done) robot.running=false; //If we are done... whatever that means, they stop the robot from running
		}
					
	}
	robot.running = true;
	robot.yDest = -1.0*robot.yDest;
	robot.y=0;
	robot.done = false;
	
}

	
	
	
	/* TESTING FOR ENCODER SIMULATION
	printf("Simulated encoder with interrupt code. Kp is only feedback implemented\n");
	getchar();
	float RPM_GOAL = 20.0;
	float Kp = 0.00002;
	float error = 0;
	float percentError = 0;
	double delay = 0.03;
	
	//toggle all encoder bits to check callback functionality
	unsigned level = LOW;
	
	for (int i=0;i<=300;i++){
		//Clears the Console Screen
		error = (motor[FRONT_MOTOR].RPM - RPM_GOAL);
		percentError = (fabs(motor[FRONT_MOTOR].RPM-RPM_GOAL)/(RPM_GOAL))*100.0;
		if (fabs(error*Kp)<=1) delay+= error*Kp;
		else (printf("Error too large %0.4f\n",(error*Kp)));

		printf("Goal RPM - %0.4f\nLast Error - %0.3f\n\n", RPM_GOAL,error);		
		printf("True RPM = %0.4f\n",motor[FRONT_MOTOR].RPM);
		printf("Percent Error %0.4f\n",percentError);
		
		temp=gpio_write(PI_VALUE, motor[FRONT_MOTOR].encA, level);
		time_sleep(delay);
				
		temp=gpio_write(PI_VALUE, motor[FRONT_MOTOR].encB, level);
		time_sleep(delay);

		if(temp != OK)printf("Something went wrong %d\n", temp);
	
		printf("\033[H\033[2J"); 
	
	
		
		if(level==LOW) level = HIGH;
		else level=LOW;
		
		
	
	}
	*/
	
	//Just to verify output pins/states of motors
	for(int i=0;i<=3;i++){
		motor[i].display();
		printf("\n");
	}
	
	printf("%d %d\n", PI_VALUE, temp);
	
	
	//release encoder callback memory allocation
	for(int i=0;i<=3;i++) motor[i].re_cancel();
	
	for(int i=0;i<=3;i++){
		temp = changePWM(motor[i].PI_VALUE, OFF, motor[i].getpwmPIN());
		if (temp != OK) printf("Motor %d failed to stop\n", motor[i].getmotor());
		else printf("Motor %d stopped\n", motor[i].getmotor());
	}
	
	piCleanup(PI_VALUE);
		
	pigpio_stop(PI_VALUE);
	for(int i=0;i<=3;i++)printf("Motor: %d\nCount: %d\n", motor[i].motor, motor[i].count);
	return PI_VALUE;
	
}
//THIS NEEDS TO BE WRAPPED IN THE ROBOT CLASS... SAME WITH MOTOR ARRAY AND WHOLE THING
/*
void Start_Drive(int direction){
	if (direction==STOP
		if(choice == STOP){
			for(int i=0;i<=3;i++){
				temp = changePWM(motor[i].PI_VALUE, OFF, motor[i].getpwmPIN());
				if (temp != OK) printf("Motor %d failed to stop\n", motor[i].getmotor());
				else printf("Motor %d stopped\n", motor[i].getmotor());
			}
			
		}
	
	
}
* */
