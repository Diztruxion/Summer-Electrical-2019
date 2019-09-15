//robot.hpp
#ifndef robot_H_
#define robot_H_

//Cardinal directions used for oritation.
//We will design so the robot always faces North at startup
//think mod 2... for some kind of logic later
#define NORTH 0
#define EAST 1
#define SOUTH 2
#define WEST 3

//Is the number of mm/rising edge of one channel of our encoder
#define DISTANCE_CONSTANT 3.547

class Robot{
	
	public:
	
		Motor* motor;
		
		void CleanUp();
		void Setup();
		void callback(int,int);
		
		Robot();
		
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
		
		
	private:

		
};


#endif




