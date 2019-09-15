//motorControl.cpp
/*
Used to setup, and for control of motors
Designed to work in the foreground, just set the speed and the speedControl
loop will deal with changing the PWM effectively.
Initial setup will have all motors OFF and will require the location passed
via constructor
*/
#include <stdio.h>
#include <pigpiod_if2.h>
#include <wiringPi.h>

#include "pwmControl.h"
#include "motorControl.h"


/*
 * MAP of Motor Location and PWM/Control with GPIO pins:
 * 
 * 
 * 
 *         			     FRONT_MOTOR[0] (DIR=5, PWM=6)
 * 
 * LEFT_MOTOR[3] (DIR=3, PWM=4)            RIGHT_MOTOR[1] (DIR=23, PWM=24)
 * 
 *         			     REAR_MOTOR[2] (DIR=25, PWM=26)
 */
//Constructor
Motor::Motor(int motor, int PI_VALUE, int dirPIN, int pwmPIN, unsigned encA, unsigned encB){
	this->setSpeed = 0; // 

	this->running = OFF;
	this->RPM = 0;
	this->Kp=0.015;//was 0.015
	this->Ki=0;
	this->Kd=0;
	this->distance=0; //Initially the motors haven't moved at all
	this->percent=0;
	this->error=0;
	this->prev_error=0;
	this->total_error=0;
	this->count=0;
	
	this->motor = motor;
	this->PI_VALUE = PI_VALUE;
	this->dirPIN = dirPIN;
	this->pwmPIN = pwmPIN;
	this->encA = encA;
	this->encB = encB;
}

//prints private member data for debugging
void Motor::display(){
	printf("Motor - %d\n", motor);
	printf("setSpeed - %0.4f\n", setSpeed);
	printf("running - %d\n", running);
	printf("dirPIN - %d\n", dirPIN);
	printf("pwmPIN - %d\n", pwmPIN);
}
//------------------------ENCODER FUNCTIONS--------------------------
void Motor::EncoderSetup(int PI_VALUE, unsigned gpioA, unsigned gpioB, re_decoderCB_t callback){
	
	
	mygpioA = gpioA;
	mygpioB = gpioB;

	mycallback = callback;
	
	prev_tick = 0;

	levA=0;
	levB=0;

	lastGpio = -1;

	set_mode(PI_VALUE, gpioA, PI_INPUT);
	//set_mode(PI_VALUE, gpioB, PI_INPUT);

	/* pull up is needed as encoder common is grounded */
	//set_pull_up_down(PI_VALUE,gpioA,PI_PUD_UP);
	//set_pull_up_down(PI_VALUE,gpioB,PI_PUD_UP);
	//gpioSetPullUpDown(gpioA, PI_PUD_UP);
	//gpioSetPullUpDown(gpioB, PI_PUD_UP);

	/* monitor encoder level changes */
	
	//set glitch filters to steady input
	
	set_glitch_filter(PI_VALUE, gpioA, STEADY);
	//set_glitch_filter(PI_VALUE, gpioB, STEADY);


	callback_A = callback_ex(PI_VALUE, gpioA, RISING_EDGE, _pulseExMotor, this);
	//callback_B = callback_ex(PI_VALUE, gpioB, EITHER_EDGE, _pulseExMotor, this);
}

//Function which does the processing for interrupt routine.
//will adjust motor speed with PID tuning loop
//Returns to callback function which will track location
void Motor::_pulseMotor(int PI_VALUE, unsigned gpio, unsigned level, uint32_t tick){
	
	/*if ((int)gpio != lastGpio){  // debounce 
		lastGpio = gpio;
		Here we have a valid tick, so increment based on direction... Fix later
		if ((gpio == mygpioA) && (level == 1)){
			if (levB) (mycallback)(1);
		}
		else if ((gpio == mygpioB) && (level == 1)){
			if (levA) (mycallback)(-1);
		}
	*/
	
	this->count++;//Add one to the motor count... will be used later for comparison to assess drift between sides.
		
	if((prev_tick!=0) &&(running)){//only adjust speed for running motors that have a previous time value
		
		//RPM calculated base on PULSE_CONSTANT using X4 encoding
		RPM = PULSE_CONSTANT/(tick-prev_tick);
		printf("Motor# %d\n", motor);
		printf("setRPM: %0.6f\n", setSpeed);
		printf("RPM: %0.6f\n\n", RPM);
		printf("Count: %d\n",this->count);
		
		
		//calculations for PID controller
		prev_error = error;
		error = (setSpeed - RPM);
		total_error += error;
		
		printf("error: %0.6f\n", error);
		printf("prev_error: %0.6f\n", prev_error);
		printf("total_error: %0.6f\n\n", total_error);
		
		//If the sign of our error has switched, remove accumulated error
		if(prev_error*error < 0) total_error = 0; 
		
		//add the PID values to the percent change in motor to adjust speed
		percent += (error*Kp);
		percent += ((prev_error-error)/(tick-prev_tick))*Kd;
		percent += (total_error*Ki);
		
		printf("percent PWM: %0.6f\n", percent);
		
		changePWM(PI_VALUE, percent, pwmPIN);
	}
	
	prev_tick = tick;
			
	mycallback(motor,running);
	
	printf("\033[H\033[2J"); //Clears Screen
}

	

void Motor::_pulseExMotor(int PI_VALUE, unsigned gpio, unsigned level, uint32_t tick, void *user){
   /*
      Need a static callback to link with C.
   */
   Motor *mySelf = (Motor *) user;

   mySelf->_pulseMotor(PI_VALUE, gpio, level, tick); /* Call the instance callback. */
}

//Release callback
void Motor::re_cancel(void){
	callback_cancel(callback_A);
	callback_cancel(callback_B);
}
//------------------------END ENCODER FUNCTIONS-----------------------

//for reading private member variables
int Motor::getdirPIN(){return dirPIN;}
int Motor::getpwmPIN(){return pwmPIN;}	
int Motor::getspdPIN(){return spdPIN;}

float Motor::getsetSpeed(){return setSpeed;}
int Motor::getmotor(){return motor;}

void Motor::changeRun(int running){this->running = running;}

int Motor::getRun(){return this->running;}

//for changing the setSpeed in the object
void Motor::changeSpeed(float setSpeed){this->setSpeed = setSpeed;}

int Motor::pinSetup(){
	
	int temp = 0;
	
	temp = set_mode(this->PI_VALUE, this->getdirPIN(), PI_OUTPUT);
	if (temp != OK) return temp;
	
	temp = initPWM(this->PI_VALUE, PWM_FREQ, this->getsetSpeed(), this->getpwmPIN());
	if (temp != OK) return temp;

	return 0;
	
}
