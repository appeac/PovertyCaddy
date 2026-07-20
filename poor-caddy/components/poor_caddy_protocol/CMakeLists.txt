idf_component_register(SRCS "src/protocol.cpp" "src/crc32.cpp" "src/control_logic.cpp" "src/sequence.cpp" "src/session_tracker.cpp" "src/ramp.cpp" "src/motor_policy.cpp" INCLUDE_DIRS "include")
target_compile_features(${COMPONENT_LIB} PUBLIC cxx_std_17)
target_compile_options(${COMPONENT_LIB} PRIVATE -Wall -Wextra -Werror)
