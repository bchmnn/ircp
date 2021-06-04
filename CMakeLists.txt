cmake_minimum_required(VERSION 3.16)
project (CC1200)

set (CMAKE_SYSTEM_NAME Generic)
set (CMAKE_SYSTEM_PROCESSOR ARM)

set (CMAKE_C_STANDARD 99)
set (CMAKE_C_STANDARD_REQUIRED ON)

set (CMAKE_CROSS_COMPILING TRUE)

set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mcpu=cortex-a8 -O2 -Wall -Wextra -Werror")
set (CMAKE_EXE_LINKER_FLAGS "-Wl,-rpath=lib")

include_directories(include)
link_directories(lib)
file(GLOB SOURCES "src/*.c")

add_executable(target ${SOURCES})

target_link_libraries(target prussdrv spi)
