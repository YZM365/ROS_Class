#include <ros/ros.h>
#include <move_base_msgs/MoveBaseAction.h>
#include <std_msgs/String.h>
#include <actionlib/client/simple_action_client.h>
#include <iostream>
#include <tf2/LinearMath/Quaternion.h>
#include <geometry_msgs/Quaternion.h>
#include <cmath>
#include <apriltag_ros/AprilTagDetectionArray.h>

using namespace std;

// 导航客户端定义
typedef actionlib::SimpleActionClient<move_base_msgs::MoveBaseAction> MoveBaseClient;

// ===================== 新增核心函数：单次AprilTag检测 + 语音播报 =====================
// 功能：到达目标点后，执行1次标签检测，并播报结果
// 参数：nh - 节点句柄；target_id - 要检测的目标标签ID
// 返回值：是否找到目标标签
bool detectAndAnnounceTag(ros::NodeHandle& nh, int target_id)
{
    // 1. 初始化语音发布者
    ros::Publisher voice_pub = nh.advertise<std_msgs::String>("/talk", 10);
    ros::Duration(0.2).sleep();  // 等待发布者初始化，防止消息丢失

    // 2. 等待接收AprilTag消息（超时3秒，避免程序卡死）
    auto tag_msg = ros::topic::waitForMessage<apriltag_ros::AprilTagDetectionArray>(
        "/tag_detections", nh, ros::Duration(3.0));

    std_msgs::String voice_msg;
    bool find_target = false;

    // 3. 未接收到标签消息
    if (!tag_msg)
    {
        ROS_WARN("not received Apriltag");
        voice_msg.data = "未接收到标签信号";
    }
    // 4. 接收到消息，遍历查找目标ID
    else
    {
        for (auto& detection : tag_msg->detections)
        {
            for (int id : detection.id)
            {
                if (id == target_id)
                {
                    find_target = true;
                    break;
                }
            }
            if (find_target) break;
        }

        // 5. 根据检测结果设置语音内容
        if (find_target)
        {
            voice_msg.data = "已找到目标";
            ROS_INFO("find AprilTag%d", target_id);
        }
        else
        {
            voice_msg.data = "未找到目标";
            ROS_INFO("not find AprilTag%d", target_id);
        }
    }

    // 6. 发布语音消息
    voice_pub.publish(voice_msg);
    return find_target;
}

// ===================== 主函数：导航逻辑 =====================
int main(int argc, char **argv)
{
    ros::init(argc, argv, "final_node");
    ros::NodeHandle nh;

    // 初始化导航客户端
    MoveBaseClient ac("move_base", true);
    ac.waitForServer();
    ROS_INFO("已连接move_base服务端！");

    tf2::Quaternion quaternion;

    // ===================== 目标点1 =====================
    move_base_msgs::MoveBaseGoal goal1;
    quaternion.setRPY(0, 0, -1.708440);
    goal1.target_pose.pose.position.x = 2.270775;
    goal1.target_pose.pose.position.y = -1.998624;
    goal1.target_pose.pose.orientation.z = quaternion.z();
    goal1.target_pose.pose.orientation.w = quaternion.w();
    goal1.target_pose.header.frame_id = "map";
    goal1.target_pose.header.stamp = ros::Time::now();

    ac.sendGoal(goal1);
    ROS_INFO("Sending Goal 1！");
    ac.waitForResult();

    if (ac.getState() == actionlib::SimpleClientGoalState::SUCCEEDED)
    {
        ROS_INFO("The Goal 1 Reached Successfully!!!");
        // ✅ 到达后：调用检测函数，执行1次标签检测+语音播报
        detectAndAnnounceTag(nh, 1);
    }
    else
    {
        ROS_WARN("The Goal Planning Failed for some reason");
    }

    // ===================== 目标点2 =====================
    move_base_msgs::MoveBaseGoal goal2;
    quaternion.setRPY(0, 0, 1.311625);
    goal2.target_pose.pose.position.x = 1.896779;
    goal2.target_pose.pose.position.y = -3.601497;
    goal2.target_pose.pose.orientation.z = quaternion.z();
    goal2.target_pose.pose.orientation.w = quaternion.w();
    goal2.target_pose.header.frame_id = "map";
    goal2.target_pose.header.stamp = ros::Time::now();

    ac.sendGoal(goal2);
    ROS_INFO("Sending Goal 2！");
    ac.waitForResult();

    if (ac.getState() == actionlib::SimpleClientGoalState::SUCCEEDED)
    {
        ROS_INFO("The Goal 2 Reached Successfully!!!");
        // ✅ 到达后：调用检测函数，执行1次标签检测+语音播报
        detectAndAnnounceTag(nh, 1);
    }
    else
    {
        ROS_WARN("The Goal Planning Failed for some reason");
    }

    // ===================== 回家目标点 =====================
    move_base_msgs::MoveBaseGoal goal3;
    quaternion.setRPY(0, 0, -0.322329);
    goal3.target_pose.pose.position.x = -1.150072;
    goal3.target_pose.pose.position.y = -2.032008;
    goal3.target_pose.pose.orientation.z = quaternion.z();
    goal3.target_pose.pose.orientation.w = quaternion.w();
    goal3.target_pose.header.frame_id = "map";
    goal3.target_pose.header.stamp = ros::Time::now();

    ac.sendGoal(goal3);
    ROS_INFO("Send Goal Home !!!");
    ac.waitForResult();

    if (ac.getState() == actionlib::SimpleClientGoalState::SUCCEEDED)
    {
        ROS_INFO("Back !!!!");
    }
    else
    {
        ROS_WARN("The Goal Planning Failed for some reason");
    }

    return 0;
}
