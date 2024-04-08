cmake_minimum_required (VERSION 3.2)

add_executable (cvidaemon "daemon.c")
#add_executable (client-test "client.c")

set(CROSS_COMPILE $ENV{CROSS_COMPILE})
set(CMAKE_C_COMPILER ${CROSS_COMPILE}gcc)
set(CMAKE_CXX_COMPILER ${CROSS_COMPILE}g++)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,-rpath,/lib/arm-linux-gnueabihf")
