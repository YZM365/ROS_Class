（前置激活：roslaunch upros_bringup bringup_w2a.launch）
惯性测量单元传感器实现：rosrun my_class_pkg ros_imu_rotate_node
激光雷达避障：rosrun my_class_pkg ros_avoid_node
建图导航：roslaunch upros_navigation navigation.launch
  roslaunch upros_navigation view_nav.launch
