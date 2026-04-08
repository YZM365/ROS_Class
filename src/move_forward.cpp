#include <ros/ros.h>
#include <geometry_msgs/Twist.h>
#include <nav_msgs/Odometry.h>
#include <tf/tf.h>
#include <cmath>

double now_x = 0.0;
double now_y = 0.0;
double now_yaw = 0.0;
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
		
int main(int argc,char **argv)
{
	ros::init(argc,argv,"square_node");
	ros::NodeHandle nh;
	
	ros::Publisher cmd_pub = nh.advertise<geometry_msgs::Twist>("/cmd_vel",10);//send Handler
	ros::Subscriber odom_sub = nh.subscribe("/odom",10,odomCallback);
	ros::Rate rate(10); //T
	ROS_INFO("SYS START:waiting");
	while (ros::ok() && now_yaw == 0.0){
		ros::spinOnce();
		rate.sleep();
	}
	ROS_INFO("start moving");
	
	double forward_speed = 0.20;
	double turn_speed = 0.50;
	double distance = 1.0;
	double angle = M_PI / 2.0;    //target settings
	
	for (int i=0;i<4;i++){       //four steps (every steps)
		double start_x = now_x;
		double start_y = now_y;
		double start_yaw = now_yaw;
		
		geometry_msgs::Twist move;
		move.linear.x = forward_speed;
		
		ROS_INFO("Moving...");
		while (ros::ok()){
			double traveled = sqrt(pow(now_x - start_x,2) + pow(now_y - start_y,2)); //cal l
			if (traveled >= distance - 0.02) break; //control
			
			cmd_pub.publish(move);
			ros::spinOnce();
			rate.sleep();
		}   //move forward for 1 m
		move.linear.x = 0;
		cmd_pub.publish(move);
		ros::Duration(0.5).sleep();
		
		ROS_INFO("Turning...");
		double target_yaw = start_yaw + angle;
		while (target_yaw > M_PI) target_yaw -= 2*M_PI;
		while (target_yaw < M_PI) target_yaw += 2*M_PI;
		
		geometry_msgs::Twist turn;turn.angular.z = turn_speed;
		
		while (ros::ok()){
			double delta = target_yaw - now_yaw;
			while (delta > M_PI) delta -= 2*M_PI;
			while (delta > M_PI) delta += 2*M_PI;
			if (fabs(delta) < 0.03) break;
			cmd_pub.publish(turn);
			ros::spinOnce();
			rate.sleep();
		}
		
		turn.angular.z = 0;
		cmd_pub.publish(turn);
		ros::Duration(0.5).sleep();   //turn
	}    //"for"
	
	//STOP
	geometry_msgs::Twist stop;
	cmd_pub.publish(stop);
	ROS_INFO("SYS FINISH");
	
	ros::shutdown();
	return 0;
}
