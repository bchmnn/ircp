cmake_minimum_required(VERSION 2.8.9)
project (CC1200)

set (CMAKE_C_STANDARD 99)
set (CMAKE_C_STANDARD_REQUIRED ON)
set (LOCAL_LIB "/usr/local/lib")
set (PRUSS_LIBS "-Wl,-rpath=$(LOCAL_LIB) -L$(LOCAL_LIB) -lprussdrv")
set (SPI_LIB "-L${LOCAL_LIB} -lspi")
set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${SPI_LIB} ${PRUSS_LIBS}")

include_directories(include)
file(GLOB SOURCES "src/*.c")

add_executable(rx_rssi "src/rx_rssi.c")
add_executable(rx_packet "src/rx_packet.c")
add_executable(tx_rssi "src/tx_rssi.c")
