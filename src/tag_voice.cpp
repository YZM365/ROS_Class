#include <ros/ros.h>
#include <move_base_msgs/MoveBaseAction.h>
#include <std_msgs/String.h>
#include <actionlib/client/simple_action_client.h>
#include <iostream>
#include <tf2/LinearMath/Quaternion.h>
#include <geometry_msgs/Quaternion.h>
#include <cmath>
#include <apriltag_ros/AprilTagDetectionArray.h>

ros::Publisher voice_pub;
// 触发开关：false=允许触发一次检测，true=锁死不操作
bool spoken_flag = false; 

// 视觉检测回调
void tagCallback(const apriltag_ros::AprilTagDetectionArray::ConstPtr& msg)
{
    // 核心：只有flag为false时，才执行单次检测（锁死时直接退出）
    if (spoken_flag)
    {
    	ROS_INFO("111111111");
        return;
    }

    // 步骤1：先假设【未找到】，遍历所有标签查找ID=1
    bool find_target = false;
    for (auto& detection : msg->detections)
    {
        for (int id : detection.id)
        {
            if (id == 1)
            {
                find_target = true;  // 找到目标，标记为true
                break;  // 找到就不用继续查了
            }
        }
        if (find_target) break;
    }

    // 步骤2：根据查找结果播报语音
    std_msgs::String voice_msg;
    if (find_target)
    {
        voice_msg.data = "已找到目标";
        ROS_INFO("AprilTag1 founded.");
    }
    else
    {
        voice_msg.data = "未找到目标";
        ROS_INFO("AprilTag1 NOT founded.");
    }
    voice_pub.publish(voice_msg);

    // 步骤3：关键！无论找到没找到，都把flag置true（单次触发完成）
    spoken_flag = true;
}

int main(int argc, char** argv)
{
    ros::init(argc, argv, "tag_voice_node");
    ros::NodeHandle nh;

    voice_pub = nh.advertise<std_msgs::String>("/talk", 10);
    ros::Subscriber tag_sub = nh.subscribe("/tag_detections", 10, tagCallback);

    ros::Time start_wait = ros::Time::now();
    while (ros::Time::now() - start_wait < ros::Duration(5.0)) {
        ros::spinOnce();  // 正常处理标签检测+语音
        ros::Rate(10).sleep();
    }
    
    ros::spin();
    return 0;
}
