project( rocks )

cmake_minimum_required(VERSION 3.13)

set( CMAKE_CXX_STANDARD 20 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )
set( CMAKE_CXX_EXTENSIONS OFF )
add_compile_options( -Wall -Wextra -pedantic -Werror -Wfatal-errors ${OpenMP_CXX_FLAGS} )

list( APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/Modules" )

find_package( dlib REQUIRED )
find_package( Qt5 COMPONENTS Core Gui Widgets REQUIRED )
find_package( Qwt REQUIRED )
find_package( OpenMP REQUIRED )

add_executable( rocks src/cli.cpp
                      src/dat.cpp
                      src/dim.cpp
                      src/cmd.cpp
                      src/io.cpp
                      src/label.cpp
                      src/pre.cpp
                      src/print.cpp
                      src/main.cpp
                      src/model.cpp
                      src/plot.cpp
                      src/score.cpp )
target_link_libraries( rocks PUBLIC stdc++fs
                                     dlib::dlib
                                     OpenMP::OpenMP_CXX
                                     Qt5::Core Qt5::Gui Qt5::Widgets
                                     ${QWT_LIBRARY} )
target_include_directories( rocks PUBLIC ${Qt5Core_INCLUDE_DIRS}
                                          ${Qt5Gui_INCLUDE_DIRS}
                                          ${Qt5Widgets_INCLUDE_DIRS}
                                          ${QWT_INCLUDE_DIR} )
target_include_directories( rocks SYSTEM PRIVATE "/home/vorac/proj/random-forest/include" )
