project( rocks )
cmake_minimum_required( VERSION 3.13 )


# Strict C++20 for all code in this project.
set( CMAKE_CXX_STANDARD 20 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )
set( CMAKE_CXX_EXTENSIONS OFF )
add_compile_options( -Wall -Wextra -pedantic -Werror -Wfatal-errors )


# Core sources; others included together with the libraries they use.
set (SRC src/cli.cpp
         src/dat.cpp
         src/dim.cpp
         src/cmd.cpp
         src/io.cpp
         src/label.cpp
         src/pre.cpp
         src/print.cpp
         src/main.cpp
         src/model.cpp
         src/score.cpp )


# Use the graphing library Qwt.
set( USE_QWT OFF CACHE STRING "Plot experimental results." )
if( USE_QWT )
    list( APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/Modules" )
    find_package( Qt5 COMPONENTS Core Gui Widgets REQUIRED )  # needed by Qwt
    find_package( Qwt REQUIRED )
    list( APPEND SRC src/plot.cpp )
    add_compile_options( ${OpenMP_CXX_FLAGS} )
    target_compile_definitions( rocks CMAKE_USE_QWT PUBLIC ON )
    message( STATUS "qwt loaded successfully.")
endif()


# Use the ML library dlib.
# Used for fundamental tasks such at command line parsing.
set( USE_DLIB OFF CACHE STRING
    "A prominent ML lib, used for this project's infrastructure, strogly recommended to keep ON." )
if( USE_DLIB )
    find_package( OpenMP REQUIRED )  # needed by dlib
    find_package( dlib REQUIRED )
    target_compile_definitions( rocks CMAKE_USE_DLIB PUBLIC ON )
    message( STATUS "dlib loaded successfully.")
endif()


# Use the CV and ML library OpenCV.
set( USE_OPENCV OFF CACHE STRING "A classical ML and CV library." )
if( USE_OPENCV )
    find_package( OpenCV REQUIRED )
    target_compile_definitions( rocks CMAKE_USE_OPENCV PUBLIC ON )
    message( STATUS "opencv loaded successfully.")
endif()


# Use the ML library Shark
set( USE_SHARK OFF CACHE STRING "A comprehensive ML library." )
if( USE_SHARK )
    find_package( Shark REQUIRED )  # random forests and more
    include( ${SHARK_USE_FILE} )
    target_compile_definitions( rocks CMAKE_USE_SHARK PUBLIC ON )
    message( STATUS "shark loaded successfully.")
endif()


# Use a random forest implementation.
set( USE_FOREST OFF CACHE STRING "A header-only random forest." )
if( USE_FOREST )
    target_compile_definitions( rocks CMAKE_USE_FOREST PUBLIC ON )
    message( STATUS "random-forest loaded successfully.")
endif()

set( THREADS_PREFER_PTHREAD_FLAG ON )
find_package( Threads REQUIRED )


add_executable( rocks "${SRC}" )
target_link_libraries( rocks PUBLIC stdc++fs
                                    Threads::Threads)


if( USE_QWT )
    target_link_libraries( rocks PUBLIC Qt5::Core
                                        Qt5::Gui
                                        Qt5::Widgets
                                        "${QWT_LIBRARY}" )
    target_include_directories( rocks PUBLIC ${Qt5Core_INCLUDE_DIRS}
                                             ${Qt5Gui_INCLUDE_DIRS}
                                             ${Qt5Widgets_INCLUDE_DIRS}
                                             ${QWT_INCLUDE_DIR} )
endif()


if( USE_DLIB )
    target_link_libraries( rocks PUBLIC  OpenMP::OpenMP_CXX
                                         dlib::dlib )
endif()


if( USE_OPENCV )
    target_link_libraries( rocks PUBLIC "${OpenCV_LIBS}" )
endif()


if( USE_SHARK )
    target_link_libraries( rocks PUBLIC "${SHARK_LIBRARIES}" )
endif()


if( USE_FOREST )
    target_include_directories( rocks SYSTEM PRIVATE "../extern/random-forest/include" )
endif()
