cmake_minimum_required(VERSION 3.16)
project(hector_ci_smoke)

find_package(ament_cmake REQUIRED)
# Only available from deb.teamhector.de (/opt/hector), not from upstream ROS
find_package(hector_ros2_utils REQUIRED)

ament_package()
