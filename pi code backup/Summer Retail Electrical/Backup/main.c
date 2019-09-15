//main.c
//sudo pigpiod -s 10 *Sets up daemon if stopped to the right sampling rate
#include <stdio.h>
#include <pigpiod_if2.h>
#include <wiringPi.h>
#include "init.h"
#include "pwmControl.h"
#include "speedControl.h"
#include "motorControl.h"
#include "piCleanup.h"


#define LOW 0
#define HIGH 1

#define OK 0

int main(){
	
	int PI_VALUE = 0;
	int temp = 0;
	//PI_VALUE = pigpio_start(NULL,NULL);
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
	
	getchar();
	*/
	//Create object array for motors (loc,PI,dirPIN, pwmPIN) as defined in motorControl.h
	//might overload Constructor later for cleaner initialization.
	Motor motor[4] = {
		Motor(FRONT_MOTOR, PI_VALUE, FRONT_MOTOR_DIR, FRONT_MOTOR_PWM),
		Motor(RIGHT_MOTOR, PI_VALUE, RIGHT_MOTOR_DIR, RIGHT_MOTOR_PWM),
		Motor(REAR_MOTOR, PI_VALUE, REAR_MOTOR_DIR, REAR_MOTOR_PWM),
		Motor(LEFT_MOTOR, PI_VALUE, LEFT_MOTOR_DIR, LEFT_MOTOR_PWM)
	};
	
	//Call Setup Function of each Motor. Done outside Constructor for future exceptions
	//Move to an init function later for cleaner code.
	for(int i=0;i<=3;i++){
		motor[i].pinSetup();		
	}
	
	//Just to verify output pins/states of motors
	for(int i=0;i<=3;i++){
		motor[i].display();
		printf("\n");
		
	}
		
	//testing PWM change on motor object array
	for(int i=0;i<=3;i++){
		motor[i].changeSpeed(i*10 + 20);//change the motor % to between 20->50 to confirm output pins
		printf("%d\n", motor[i].getsetSpeed());
		temp = changePWM(motor[i].PI_VALUE, motor[i].getsetSpeed(), motor[i].getpwmPIN());
		
		if (temp != OK) printf("Motor %d failed to start\n", motor[i].getmotor());
		temp = OK;
	}
	getchar();
	
	
	//Varry a motor 0 PWM pin over 0-100% PWM

	double delay=0.05;
	
	for(int i=0;i<=100;i++){
		motor[0].changeSpeed(i);
		temp = changePWM(motor[0].PI_VALUE, motor[0].getsetSpeed(), motor[0].getpwmPIN());
		
		if (temp != OK) printf("Motor %d failed to start\n", motor[0].getmotor());
		temp = OK;
		
		time_sleep(delay);
		
	}
	
	for(int i=100;i>=0;i--){
		motor[0].changeSpeed(i);
		temp = changePWM(motor[0].PI_VALUE, motor[0].getsetSpeed(), motor[0].getpwmPIN());
		
		if (temp != OK) printf("Motor %d failed to start\n", motor[0].getmotor());
		temp = OK;
		
		time_sleep(delay);
		
	}
	
	printf("%d %d", PI_VALUE, temp);
	
	piCleanup(PI_VALUE);
		
	pigpio_stop(PI_VALUE);
	return PI_VALUE;
	
}
