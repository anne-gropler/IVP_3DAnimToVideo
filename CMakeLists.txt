cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

set(META_PROJECT_NAME "ivp")
set(target ivp)
project(${META_PROJECT_NAME})

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5Core)
find_package(OpenSceneGraph COMPONENTS osgViewer osgGA osgDB osgUtil osgAnimation REQUIRED)
find_package(OpenGL REQUIRED)
find_package(OsgWorks REQUIRED)


set(IVP_SOURCE 
	src/main.cpp
	src/modelviewerqt.ui
	src/modelviewerqt.h
	src/modelviewerqt.cpp
	src/viewerosg/osgadapterwidget.h
	src/viewerosg/osgadapterwidget.cpp
	src/viewerosg/osggraphicswindowqt.h
	src/viewerosg/osggraphicswindowqt.cpp
	src/viewerosg/viewerqtcgs.cpp
	src/viewerosg/viewerqtcgs.h
	src/SceneGraphVisitor.cpp
	src/SceneGraphVisitor.h
	)

add_executable(${target}
		${IVP_SOURCE})

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}  
	${CMAKE_CURRENT_BINARY_DIR} 
	${PROJECT_SOURCE_DIR}
	${OPENSCENEGRAPH_INCLUDE_DIRS}
	${OPENGL_INCLUDE_DIR}
	${OSGWORKS_INCLUDE_DIRS}
	)

LINK_DIRECTORIES(
	${OSGWORKS_LIBRARY_DIR}
	)

target_link_libraries(${target}
    ${OPENSCENEGRAPH_LIBRARIES}
	${OSGWORKS_LIBRARY_DIRS}
	)

	
#qt5_auto_wrap(IVP_SOURCE ${IVP_SOURCE})

	
qt5_use_modules(ivp Core Gui OpenGL Widgets)
qt5_wrap_ui(${PROJECT_NAME} src/modelviewerqt.ui)


# Deployment
#install(TARGETS ${target}
#	RUNTIME DESTINATION .
#	)

#install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data 
#		DESTINATION .
#)
#include(cmake/cpack-config.cmake)

