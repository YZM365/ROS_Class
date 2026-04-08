#include <ros/ros.h>
#include <std_msgs/String.h>
int main(int argc,char **argv)
{
	ros::init(argc,argv,"publisher");
	ros::NodeHandle ph;
	ros::Publisher pub = ph.advertise<std_msgs::String>("test_topic",10);
	ros::Rate rate(1.0);
	while (ros::ok()){
		std_msgs::String test_msg;
		test_msg.data = "Hello,world!";
		pub.publish(test_msg);
		rate.sleep();
	}
	return 0;
}
