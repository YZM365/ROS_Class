#include <ros/ros.h>
#include <std_msgs/String.h>
#include <apriltag_ros/AprilTagDetectionArray.h>

ros::Publisher voice_pub;
bool spoken_flag = false; // 只播报一次

// 视觉检测回调
void tagCallback(const apriltag_ros::AprilTagDetectionArray::ConstPtr& msg)
{
    // 遍历所有检测到的标签
    for (auto& detection : msg->detections)
    {
        for (int id : detection.id)
        {
            // 只识别①号码：ID=1
            if (id == 1 && !spoken_flag)
            {
                // 语音播报
                std_msgs::String voice_msg;
                voice_msg.data = "已找到目标一";
                voice_pub.publish(voice_msg);

                ROS_INFO("检测到 AprilTag1，已播报");
                spoken_flag = true;
                return;
            }
        }
    }
}

int main(int argc, char** argv)
{
    ros::init(argc, argv, "tag_voice_node");
    ros::NodeHandle nh;

    // 语音发布
    voice_pub = nh.advertise<std_msgs::String>("/talk", 10);
    // 订阅AprilTag
    ros::Subscriber tag_sub = nh.subscribe("/tag_detections", 10, tagCallback);

    ros::spin();
    return 0;
}
