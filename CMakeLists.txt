cmake_minimum_required(VERSION 2.8)
project(QGLUT)

set(LIBS)

include_directories(${QGLUT_SOURCE_DIR}/include)	

# OpenGL
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_PATH})
set(LIBS ${LIBS} ${OPENGL_LIBRARIES})

# GLUT
find_package(GLUT REQUIRED)
include_directories(${GLUT_INCLUDE_PATH})
set(LIBS ${LIBS} ${GLUT_LIBRARIES})
	
add_subdirectory(src)
