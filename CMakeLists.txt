cmake_minimum_required(VERSION 3.0.2)
project(mesobot_project11)

find_package(catkin REQUIRED COMPONENTS
geographic_msgs
roscpp
roslint
rospy
rqt_gui
rqt_gui_cpp
sonardyne_msgs
)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
  set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

find_package(Qt5 COMPONENTS Widgets REQUIRED)

roslint_cpp()

install(DIRECTORY launch/ DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}/launch)
install(DIRECTORY config/ DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}/config)

set(rqt_mesobot_SRCS
  src/mesobot_plugin.cpp
)

set(rqt_mesobot_HDRS
  include/mesobot_project11/mesobot_plugin.h
)

set(rqt_mesobot_UIS
  src/mesobot_plugin.ui
)

set(rqt_mesobot_INCLUDE_DIRECTORIES
  include
  "${CATKIN_DEVEL_PREFIX}/${CATKIN_GLOBAL_INCLUDE_DESTINATION}"
)
if(NOT EXISTS "${CATKIN_DEVEL_PREFIX}/${CATKIN_GLOBAL_INCLUDE_DESTINATION}")
  file(MAKE_DIRECTORY "${CATKIN_DEVEL_PREFIX}/${CATKIN_GLOBAL_INCLUDE_DESTINATION}")
endif()

catkin_package(
  INCLUDE_DIRS ${rqt_mesobot_INCLUDE_DIRECTORIES}
  LIBRARIES ${PROJECT_NAME}
  CATKIN_DEPENDS roscpp rqt_gui_cpp rqt_gui sonardyne_msgs
#  DEPENDS system_lib
)

catkin_python_setup()

qt5_wrap_cpp(rqt_mesobot_MOCS ${rqt_mesobot_HDRS})

set(_cmake_current_binary_dir "${CMAKE_CURRENT_BINARY_DIR}")
set(CMAKE_CURRENT_BINARY_DIR "${CATKIN_DEVEL_PREFIX}/${CATKIN_GLOBAL_INCLUDE_DESTINATION}")
qt5_wrap_ui(rqt_mesobot_UIS_H ${rqt_mesobot_UIS})
set(CMAKE_CURRENT_BINARY_DIR "${_cmake_current_binary_dir}")

include_directories(
    ${rqt_mesobot_INCLUDE_DIRECTORIES}
    ${catkin_INCLUDE_DIRS}
)

add_library(${PROJECT_NAME}
    ${rqt_mesobot_SRCS}
    ${rqt_mesobot_MOCS}
    ${rqt_mesobot_UIS_H}
)

qt5_use_modules(${PROJECT_NAME} Widgets)

target_link_libraries(${PROJECT_NAME}
    ${QT_LIBRARIES}
    ${catkin_LIBRARIES}
)

find_package(class_loader)
class_loader_hide_library_symbols(${PROJECT_NAME})


install(FILES
  plugin.xml
  DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}
)

install(TARGETS ${PROJECT_NAME}
  ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
  LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
  RUNTIME DESTINATION ${CATKIN_GLOBAL_BIN_DESTINATION}
)

# catkin_install_python(PROGRAMS scripts/rqt_sonardyne_ranger
#   DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
# )

install(DIRECTORY include/${PROJECT_NAME}/
  DESTINATION ${CATKIN_PACKAGE_INCLUDE_DESTINATION}
)
