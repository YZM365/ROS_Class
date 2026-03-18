#include "ros/ros.h"
 
 int main(int argc,char **argv){
 	ros::init(argc,argv,"cre_node");
 	ROS_INFO("Hello ROS C++ !!!");
 	ros::MultiThreadedSpinner spinner;
 	spinner.spin();
 }
