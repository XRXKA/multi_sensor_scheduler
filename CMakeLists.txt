cmake_minimum_required(VERSION 3.10)
project(multi_sensor_scheduler)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

set(SOURCE_FILES
    src/main.cpp
    src/ring_buffer.cpp
    src/scheduler.cpp
    src/sync_manager.cpp
    src/metrics.cpp
    src/logger.cpp
)

add_executable(sensor_scheduler ${SOURCE_FILES})

target_link_libraries(sensor_scheduler pthread)
