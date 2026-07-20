cmake_minimum_required(VERSION 3.16)
project(poor_caddy_host_tests CXX)
set(CMAKE_CXX_STANDARD 17)
add_executable(poor_caddy_tests
 ../components/poor_caddy_protocol/test/test_protocol.cpp
 ../components/poor_caddy_protocol/test/test_control.cpp
 ../components/poor_caddy_protocol/test/test_motor_policy.cpp
 test_odrive_protocol.cpp
 ../components/poor_caddy_protocol/src/protocol.cpp
 ../components/poor_caddy_protocol/src/crc32.cpp
 ../components/poor_caddy_protocol/src/control_logic.cpp
 ../components/poor_caddy_protocol/src/sequence.cpp
 ../components/poor_caddy_protocol/src/session_tracker.cpp
 ../components/poor_caddy_protocol/src/ramp.cpp
 ../components/poor_caddy_protocol/src/motor_policy.cpp
 ../cart/main/odrive_protocol.cpp)
target_include_directories(poor_caddy_tests PRIVATE ../components/poor_caddy_protocol/include ../cart/main)
target_compile_options(poor_caddy_tests PRIVATE -Wall -Wextra -Werror)
enable_testing()
add_test(NAME poor_caddy_tests COMMAND poor_caddy_tests)
