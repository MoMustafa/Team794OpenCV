cmake_minimum_required (VERSION 2.8)

project(mainprogram)

find_package(OpenCV REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS} includes)

file(GLOB SOURCES "src/*.cpp")

add_executable(main ${SOURCES})

target_link_libraries(main ${OpenCV_LIBS})
