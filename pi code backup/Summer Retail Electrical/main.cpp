//main.c
//sudo pigpiod -s 10 *Sets up daemon if stopped to the right sampling rate
#include <stdio.h>
#include <csignal>
#include <cmath>
#include <cstring>
#include <pigpiod_if2.h>
#include <wiringPi.h>
#include <fstream>
#include "init.h"
#include "pwmControl.h"
#include "motorControl.h"
#include "piCleanup.h"
#include "robot.hpp"
#include "waypoint.hpp"
#include <pthread.h>
#include <curl/curl.h>


//takes the camera call command and the waypoint

//void *take_pic(int cameraIP, int waypoint);
void take_pictures(int picNum);

//global and statics for curl multithreading
//Taken from https://curl.haxx.se/libcurl/c/multithread.html



/*IMPLEMENT LATER... struct field for data for threads
struct CurlData{
	const char *url;
	char postData[50];
};
*/
#define NUMTHREAD 3

char postData[40];
static int timeLapse=0;


const char * const urls[NUMTHREAD]= {
  "http://192.168.137.101",
  "http://192.168.137.103",
  "http://192.168.137.105",
};
/*
const char * const urls[NUMTHREAD]= {
  "http://192.168.137.101"
};
*/


static void *pull_one_url(void *url){
	
	CURL *curl;
	
	curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData);
	printf("%s\n",(char*)url);
	printf("%s\n",(char*)postData);
	
	//avoid executing curl for now.
	//return NULL;
	
	curl_easy_perform(curl); /* ignores error */ 
	curl_easy_cleanup(curl);
	
	return NULL;
}

#define OK 0

#define IMGSPACE_CONST 170 //we want 8images/1m or 1-image/220mm so calculate wp based on this 

static int PI_VALUE = 0;
static Robot robot;

//callback function Only gets called for Front/Right Motor
//Will use for navigation. Ignore if running is false
void drive_angle(int speed, int angle);

void callback(int motor, int running){
	static int times=0;
	times++;
	
	static bool locked = false;
	
	//only process for running motors... Might add some compensation later for not running interrupts
	if(robot.running){
		while(locked); //spin-lock to avoid simultaneous access
		locked = true;//lock function
			
		if(((motor==RIGHT_MOTOR)||(motor==FRONT_MOTOR))&&(running==ON)){//only register positional updates on right or front
			if (robot.dy!=0){
				robot.y+=robot.dy;
				//Might change, but basically if the difference is <= 5mm. Stop
				if (fabs(robot.y-robot.yDest)<=DISTANCE_OFFSET) robot.stopping = true;
			}
			else if(robot.dx !=0){
				robot.x+=robot.dx;
				//Might change, but basically if the difference is <= 5mm. Stop
				if (fabs(robot.x-robot.xDest)<=DISTANCE_OFFSET) robot.stopping = true;
			}
		}
		/*
				
		y and x position tracking and next destination values
		distance travelled is derived from tick count on tire totation
		Circumference of 319.19mm/90 ticks in X1 mode = 3.547mm/tick
		Stored in DISTANCE_CONSTANT define
			
		Float position of Robot. Processed in mm. Is measured as distance from 'home'
		North[Front at start] (y)+, East[Right at start] (x)+
		*/

		if(running) robot.count++; //only increment if the motor was supposed to be running
		
		//if(robot.count>3000) robot.running=false;
		
		printf("count: %d\n", robot.count);
		printf("Y-position: %0.2f\n", robot.y);
		printf("X-position: %0.2f\n", robot.x);
		
		locked = false; //unlock function
	}
	printf("Times: %d\n", times);
}

void signal_handler(int signal_num ) { 
  //kill everything
  
   //Set all pins as LOW and Output as per manufacturer direction.
	piCleanup(PI_VALUE);
	//Stop daemon connection
	pigpio_stop(PI_VALUE);
	exit(-1);

} 
 

int main(){
	//threaded http commands to php servers, and initiate curl
	pthread_t tid[NUMTHREAD];
	curl_global_init(CURL_GLOBAL_ALL);
		
	signal(SIGTERM, signal_handler);
	
	//throw this into a function later... maybe embed into the robot class
	int numWP = 0;
	int curWP = 0;
	int temp = 0;
	
	FILE *infile;
	//infile = fopen("waypointlist.txt","r");
	infile = fopen("movetest.txt", "r");
	fscanf(infile, "%d",&numWP);
	
	
	Waypoint * waypoints = new Waypoint[numWP];
	
	//read in waypoint data and verify
	for(int i=0;i<numWP;i++){
		fscanf(infile,"%f %f %d %d", &waypoints[i].x, &waypoints[i].y, &waypoints[i].dir, &waypoints[i].pic);
		printf("Waypoint %d X:%0.1f Y:%0.1f Dir:%d Pic:%d\n", i, waypoints[i].x, waypoints[i].y, waypoints[i].dir, waypoints[i].pic);
	}

	fclose(infile);
	//getchar();

	PI_VALUE = init();
	
	if (PI_VALUE < 0){
		printf("Something didn't start check if daemon running\n");
		return PI_VALUE;
	}
		
	robot.setPI(PI_VALUE);
	robot.Setup(callback);
	
	robot.running=true;
	
	
	while(robot.running){
		
		if (!robot.moving){//if we aren't currently moving

			robot.Drive(waypoints[curWP]);// This drives to the next way point.
			
			if(waypoints[curWP].pic==true){ //turn on cameras
				
				//set timeLapse to the distance between our current waypoint and the next... risky for seg faults
				//MUST have last waypoint with no pic... aka turn off camera
				
				timeLapse = (int)(fabs((waypoints[curWP+1].x-waypoints[curWP].x)) + 
						          fabs((waypoints[curWP+1].y-waypoints[curWP].y)));
				
				//Used if sending the amount of time
				//timeLapse *= TIMELAPSE_CONST;
				timeLapse = ceil(timeLapse/IMGSPACE_CONST);
				
				//build POST payload for threads 
				sprintf(postData,"action=picture&time=%d",timeLapse);
				
				//setup threads to start
				for(int i = 0; i< NUMTHREAD; i++) {
					int error = pthread_create(&tid[i], NULL, pull_one_url, (void *)urls[i]);
					
					if(0 != error) fprintf(stderr, "Couldn't run thread number %d, errno %d\n", i, error);
					else fprintf(stderr, "Thread %d, gets %s\n", i, urls[i]);
				
				}
				//getchar();
			}
		}
		
	/*old imagery routine for WP based imagery
			printf("curWP: %d numPic:%d\n",curWP, numPic);
			time_sleep(1);
			
			take_pictures(numPic+1);
			
			numPic++;
	*/
			
		curWP++; //move to next WP
		if (curWP == numWP) robot.running = false; //If we just dealt with our last waypoint	
	
	}

	//Cleanup happens here
	 
	printf("%d %d\n", PI_VALUE, temp);

	//Set all pins as LOW and Output as per manufacturer direction.
	piCleanup(PI_VALUE);
	
	for(int i=0;i<=3;i++)printf("Motor: %d\nCount: %d\n", robot.motor[i].motor, robot.motor[i].count);
	
	//clear memory
	delete[] waypoints;
	//Stop daemon connection
	pigpio_stop(PI_VALUE);

	curl_global_cleanup();//required by curl
	//return PI_VALUE;
	return 0;
	
}


//takes all photos using waypoint int
void take_pictures(int picNum){
	char message[1024];
	sprintf(message,"curl -d \"action=picture&waypoint=%d\" http://192.168.137.101& \
					 curl -d \"action=picture&waypoint=%d\" http://192.168.137.103& \
					 curl -d \"action=picture&waypoint=%d\" http://192.168.137.105",picNum,picNum,picNum);
					 
					 
	system(message);
}
