//waypoint.hpp
//defines the waypoint structure

#ifndef waypoint_HPP_
#define waypoint_HPP_

//We will initialize WP data as 0. This will have the affect that any non specific WP will be home
	struct Waypoint{
		float x=0;
		float y=0;
		int dir=0;
		int pic=0; //picture at waypoint, or no		
		
	};

#endif
