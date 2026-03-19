#include <ros/ros.h>
#include <geometry_msgs/Twist.h>
#include <nav_msgs/Odometry.h>
#include <tf/tf.h>
#include <cmath>

double now_x = 0.0;
double now_y = 0.0;
double now_yaw = 0.0;
int dir = 0;
void odomCallback(const nav_msgs::Odometry::ConstPtr& msg)
{
	now_x = msg->pose.pose.position.x;
	now_y = msg->pose.pose.position.y;
	
	tf::Quaternion q(
		msg->pose.pose.orientation.x,
		msg->pose.pose.orientation.y,
		msg->pose.pose.orientation.z,
		msg->pose.pose.orientation.w);
		
	double roll,pitch,yaw;
	tf::Matrix3x3(q).getRPY(roll,pitch,yaw);
	now_yaw = yaw;
}
void bumpCallback(const std_msgs::Int16MultiArray::ConstPtr& msg){
		dir = 0;
		for (int i = 0;i<3;i++){
			if (msg->data[i]) {
				dir = i+1;
				//ROS_INFO("Obstacle at dir %d",dir);
				return;
			}
		}

}
		
int main(int argc,char **argv)
{
	ros::init(argc,argv,"avoid_node");
	ros::NodeHandle nh;
	
	ros::Publisher cmd_pub = nh.advertise<geometry_msgs::Twist>("/cmd_vel",10);//send Handler
	ros::Subscriber odom_sub = nh.subscribe("/odom",10,odomCallback);
	ros::Subscriber bump_sub = nh.subscribe("/robot/bump_sensor",10,bumpCallback);
	ros::Rate rate(10); //T
	ROS_INFO("SYS START:waiting for data");
	while (ros::ok() && now_yaw == 0.0){
		ros::spinOnce();
		rate.sleep();
	}
	ROS_INFO("<Mission start>");    //all sensor data been loaded
	
	double forward_speed = 0.20;
	double turn_speed = 0.50;
	double angle = M_PI;    //target angle settings(180)
	geometry_msgs::Twist stop;
	
	while (ros::ok() and not dir){  //no obstacles
		geometry_msgs::Twist move;
		ROS_INFO("Moving...");
		move.linear.x = forward_speed;
		cmd_pub.publish(move);
		ros::Duration(0.5).sleep();
		ros::spinOnce();
		
		if (dir){     //coming with obstacles
			ROS_INFO("avoiding crash from dir %d: Turning...",dir);
			cmd_pub.publish(stop);
			ros::Duration(0.2).sleep();
			double start_yaw = now_yaw;
			double target_yaw = start_yaw + angle;
			//while (target_yaw > M_PI) target_yaw -= 2*M_PI;
			//while (target_yaw < M_PI) target_yaw += 2*M_PI;
			target_yaw = atan2(sin(target_yaw),cos(target_yaw));  //repair yaw to standard angle
		
			geometry_msgs::Twist turn;turn.angular.z = turn_speed;
		
			while (ros::ok()){
				double delta = target_yaw - now_yaw;
				//while (delta > M_PI) delta -= 2*M_PI;
				//while (delta > M_PI) delta += 2*M_PI;
				delta = atan2(sin(delta),cos(delta));
				if (fabs(delta) < 0.03){
					cmd_pub.publish(stop);
					ROS_INFO("Turning complete! Continue movement");
					break;
				}
				cmd_pub.publish(turn);
				ros::spinOnce();
			}
		}   //turning loop ends(or without turning)
		rate.sleep();
	}
	rate.sleep();
	//STOP
	//geometry_msgs::Twist stop;
	//cmd_pub.publish(stop);
	//ROS_INFO("SYS FINISH");
	
	//ros::shutdown();
	return 0;
}
