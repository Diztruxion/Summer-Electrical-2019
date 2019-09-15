//robot.cpp
//Contains functions for the robots operations and status information
#include "robot.hpp"

Robot::Robot(){
	this->x = 0;
	this->y = 0;
	this->xDest = 0;
	this->yDest = 0;
	
}

Robot::Setup(){
	Motor* motor = new Motor[4] = {
			Motor(FRONT_MOTOR, PI_VALUE, FRONT_MOTOR_DIR, FRONT_MOTOR_PWM, FRONT_MOTOR_ENC_A, FRONT_MOTOR_ENC_B),
			Motor(RIGHT_MOTOR, PI_VALUE, RIGHT_MOTOR_DIR, RIGHT_MOTOR_PWM, RIGHT_MOTOR_ENC_A, RIGHT_MOTOR_ENC_B),
			Motor(REAR_MOTOR, PI_VALUE, REAR_MOTOR_DIR, REAR_MOTOR_PWM, REAR_MOTOR_ENC_A, REAR_MOTOR_ENC_B),
			Motor(LEFT_MOTOR, PI_VALUE, LEFT_MOTOR_DIR, LEFT_MOTOR_PWM, LEFT_MOTOR_ENC_A, LEFT_MOTOR_ENC_B)
	};

}

Robot::CleanUp(){
	delete[] motor;
}

Robot::callback(){
	
}
