project( rocks )

cmake_minimum_required(VERSION 3.13)

set( CMAKE_CXX_STANDARD 20 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )
set( CMAKE_CXX_EXTENSIONS OFF )
add_compile_options( -Wall -Wextra -pedantic -Werror -Wfatal-errors ${OpenMP_CXX_FLAGS} )


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
set( USE_QWT OFF CACHE STRING "Plot experiment results.")
function( load_qwt )
    if( USE_QWT )
        list( APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/Modules" )
        find_package( Qt5 COMPONENTS Core Gui Widgets )  # needed by Qwt
        find_package( Qwt )
        if( NOT Qwt_FOUND )
            message( ERROR "Qwt requested but not found!")
        endif()
        list( APPEND SRC src/plot.cpp )
    endif()
endfunction()
load_qwt()


# Use the ML library dlib.
find_package( OpenMP REQUIRED )  # needed by dlib
find_package( dlib REQUIRED )


# Use the CV and ML library OpenCV.
find_package( OpenCV REQUIRED )

# Use the ML library Shark
find_package( Shark REQUIRED )  # random forests and more
include( ${SHARK_USE_FILE} )


add_executable( rocks "${SRC}" )


target_link_libraries( rocks PUBLIC stdc++fs
                                     dlib::dlib
                                     ${OpenCV_LIBS}
                                     OpenMP::OpenMP_CXX
                                     Qt5::Core Qt5::Gui Qt5::Widgets
                                     ${QWT_LIBRARY}
                                     ${SHARK_LIBRARIES} )
target_include_directories( rocks PUBLIC ${Qt5Core_INCLUDE_DIRS}
                                          ${Qt5Gui_INCLUDE_DIRS}
                                          ${Qt5Widgets_INCLUDE_DIRS}
                                          ${QWT_INCLUDE_DIR} )
target_include_directories( rocks SYSTEM PRIVATE "../random-forest/include" )




if( USE_QWT )
    target_link_libraries( rocks PUBLIC Qt5::Core
                                        Qt5::Gui
                                        Qt5::Widgets
                                        "${QWT_LIBRARY}"
                         )
    target_include_directories( rocks PUBLIC "${Qt5Core_INCLUDE_DIRS}" )
endif()

