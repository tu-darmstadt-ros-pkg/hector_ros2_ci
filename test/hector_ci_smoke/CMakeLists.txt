cmake_minimum_required(VERSION 3.16)
project(hector_ci_smoke)

find_package(ament_cmake REQUIRED)
# Only available from deb.teamhector.de (/opt/hector), not from upstream ROS
find_package(hector_ros2_utils REQUIRED)

if(BUILD_TESTING)
  find_package(ament_cmake_gtest REQUIRED)
  ament_add_gtest(test_smoke test/test_smoke.cpp)
endif()

ament_package()
