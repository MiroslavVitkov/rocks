project( rocks )

cmake_minimum_required(VERSION 3.13)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
add_compile_options( -Wall -Wextra -pedantic -Werror -Wfatal-errors )
find_package( dlib REQUIRED )

add_executable( rocks src/cli.cpp src/cmd.cpp src/io.cpp src/main.cpp src/model.cpp src/score.cpp )
target_link_libraries( rocks stdc++fs dlib::dlib )
target_include_directories( rocks PRIVATE "../accuracy-evaluation-cpp/src")
