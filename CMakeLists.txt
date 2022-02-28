cmake_minimum_required(VERSION 3.0.2)
project(tests_alberto)

set(CMAKE_CXX_STANDARD 17)

find_package(catkin REQUIRED COMPONENTS
  geometry_msgs
  roscpp
  roslint
  std_msgs
  costmap_2d
)
catkin_package(
)

include_directories(
  include
  ${catkin_INCLUDE_DIRS}
)

add_library(${PROJECT_NAME}
   src/${PROJECT_NAME}/FakeLaser.cpp
)
add_dependencies(${PROJECT_NAME} ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})

add_executable(my_tests_node src/my_tests_node.cpp)
add_dependencies(my_tests_node ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})
target_link_libraries(my_tests_node ${catkin_LIBRARIES} ${PROJECT_NAME})

add_executable(sampling src/tests_alberto/Sampling.cpp)
add_dependencies(sampling ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})
target_link_libraries(sampling ${catkin_LIBRARIES} ${PROJECT_NAME})
roslint_cpp()
catkin_add_gtest(test1 tests/test1.cpp)
add_dependencies(test1 ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})
if(TARGET test1)
  target_link_libraries(test1 ${catkin_LIBRARIES} ${PROJECT_NAME})
endif()

## Add folders to be run by python nosetests
# catkin_add_nosetests(test)
