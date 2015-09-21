cmake_minimum_required(VERSION 2.9)

set(PROJECT_NAME VKDrone)
set(EXEC_NAME vkdrone)
project(${PROJECT_NAME} C CXX)

set(VKD_SRC
    ${CMAKE_SOURCE_DIR}/src/main.cpp
    ${CMAKE_SOURCE_DIR}/src/VKDrone.cpp
    ${CMAKE_SOURCE_DIR}/src/PID.cpp
    ${CMAKE_SOURCE_DIR}/src/BotBase.cpp
    ${CMAKE_SOURCE_DIR}/src/bot_interface.cpp
)

set(VKD_HEADERS
    ${CMAKE_SOURCE_DIR}/src/VKDrone.h
    ${CMAKE_SOURCE_DIR}/src/PID.h
    ${CMAKE_SOURCE_DIR}/src/Point2D.h
    ${CMAKE_SOURCE_DIR}/src/BotBase.h
    ${CMAKE_SOURCE_DIR}/src/bot_interface.h
)

add_executable(vkdrone ${VKD_SRC} ${VKD_HEADERS})

set_target_properties(
    ${EXEC_NAME} PROPERTIES
    COMPILE_FLAGS "-Wall -pedantic -std=c++11"
)

