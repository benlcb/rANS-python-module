#CMAKE == super cool

cmake_minimum_required (VERSION 2.8.11)
project (rANSCoder)

SET(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "/usr/local/include/boost/")
SET(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "/usr/lib/x86_64-linux-gnu/")
#SET(BOOST_ROOT ${CMAKE_INCLUDE_PATH} "/usr/lib/boost_1_75_0/")

set(PYTHON_INCLUDE_DIRS ${PYTHON_INCLUDE_DIRS} /usr/local/lib/python3.8/dist-packages/numpy/core/include/)
include_directories(${PYTHON_INCLUDE_DIRS})

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fext-numeric-literals ")
elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -fext-numeric-literals ")
else()
    message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 ")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -O3 ")

FIND_PACKAGE(PythonInterp 3.6  REQUIRED)
FIND_PACKAGE(PythonLibs 3.6  REQUIRED)
FIND_PACKAGE(Boost COMPONENTS python38 numpy38)

INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS} )
INCLUDE_DIRECTORIES( ${PYTHON_INCLUDE_DIRS} )
INCLUDE_DIRECTORIES( ${PythonInterp_INCLUDE_DIRS} )
INCLUDE_DIRECTORIES( ${NUMPY_INCLUDE_DIR} )
message("Include dirs of Python: " ${PYTHON_INCLUDE_DIRS} "(" ${PYTHONLIBS_VERSION_STRING}  ")")

message( "Boost Libraries: " ${Boost_LIBRARIES} )
set( CMAKE_BUILD_TYPE Release )


add_library(rANSCoder SHARED rANSCoder.cpp )
target_include_directories(rANSCoder PUBLIC .)
PYTHON_ADD_MODULE(pyrANS pyrANS.cpp)
TARGET_LINK_LIBRARIES(pyrANS LINK_PRIVATE ${Boost_LIBRARIES} rANSCoder )


include_directories(.)
add_executable(test
        main.cpp
        rans64_custom.hpp rANSCoder.cpp rANSCoder.h)




  


