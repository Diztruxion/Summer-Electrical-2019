//robot.hpp
#ifndef robot_H_
#define robot_H_

#include "motorControl.h"
#include "waypoint.hpp"

//Cardinal directions used for oritation.
//We will design so the robot always faces North at startup
//think mod 2... for some kind of logic later
#define LOW 0
#define HIGH 1

//Cardinal directions for orientation of robot
#define NORTH 0
#define EAST 1
#define SOUTH 2
#define WEST 3

//direction defines for Move function
#define DIR_FWD 0
#define DIR_RIGHT 1
#define DIR_REV 2
#define DIR_LEFT 3
#define DIR_ROTATE_CCW 4
#define DIR_ROTATE_CW 5
#define DIR_STOP -1

#define INITIAL_SPEED 15 //Duty cycle out of 100 (from 15-20)
#define RPM 18 //default is 20... (measured in RPM) Used to change speed while moving

//Is the number of mm/rising edge of one channel of our encoder... DONT TOUCH
#define DISTANCE_CONSTANT 3.547
#define DISTANCE_OFFSET 2.5

#define PAUSE 1.5

class Robot{
	
	public:
	
		Motor motor[4]{
				Motor(FRONT_MOTOR, PI_VALUE, FRONT_MOTOR_DIR, FRONT_MOTOR_PWM, FRONT_MOTOR_ENC_A, FRONT_MOTOR_ENC_B),
				Motor(RIGHT_MOTOR, PI_VALUE, RIGHT_MOTOR_DIR, RIGHT_MOTOR_PWM, RIGHT_MOTOR_ENC_A, RIGHT_MOTOR_ENC_B),
				Motor(REAR_MOTOR, PI_VALUE, REAR_MOTOR_DIR, REAR_MOTOR_PWM, REAR_MOTOR_ENC_A, REAR_MOTOR_ENC_B),
				Motor(LEFT_MOTOR, PI_VALUE, LEFT_MOTOR_DIR, LEFT_MOTOR_PWM, LEFT_MOTOR_ENC_A, LEFT_MOTOR_ENC_B)
		};
		
		void CleanUp();
		void Setup(re_decoderCB_t callback);

		void EncoderInit(re_decoderCB_t callback);
		void EncoderCancel();
		
		void Drive(Waypoint);
		
		void setPI(int PI_VALUE);
		void Move(int);
		
		re_decoderCB_t membercallback;
		

		int PI_VALUE;
		
		Robot();
		~Robot();
		
		int count=0; //something about ticks
		//what direction are we facing... might always stay with front facing North for now.
		//Why turn when you don't need to... Until we need to.
		int direction = NORTH;
		
		bool running = false;
		bool done = false;
		bool moving = false;
		bool stopping = false;
	
		/*
		y and x position tracking and next destination values
		distance travelled is derived from tick count on tire totation
		Circumference of 319.19mm/90 ticks in X1 mode = 3.547mm/tick
		
		*/
		
		//Float position of Robot. Processed in mm. Is measured as distance from 'home'
		//North[Front at start] (y)+, East[Right at start] (x)*
	
		float y, x;
		float yDest, xDest;
		float dx, dy;
		
	private:

		
};


#endif




