#!/usr/bin/env python3
import rospy
from std_msgs.msg import String

def callback(data):
	rospy.loginfo(rospy.get_caller_id() + "I heard %s",data.data)
def my_subscriber():
	rospy.init_node('my_subscriber',anonymous = True)
	rospy.Subscriber("my_topic",String,callback)
	rospy.spin()
if __name__ == '__main__':
	try:
		my_subscriber()
	except rospy.ROSInterruptException:
		pass
