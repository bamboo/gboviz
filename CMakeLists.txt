cmake_minimum_required(VERSION 3.10)

project(gboviz VERSION 1.0)

# Set the C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add the executable
add_executable(gboviz src/main.cpp src/simdjson.cpp src/processor.cpp)

# Set optimization flags for fast execution
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(gboviz PRIVATE -Ofast)
elseif(MSVC)
    target_compile_options(gboviz PRIVATE /O2)  # Best option for MSVC equivalent to -Ofast
endif()

# Download Google Test
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/release-1.12.1.tar.gz
  DOWNLOAD_EXTRACT_TIMESTAMP true
)
FetchContent_MakeAvailable(googletest)

# Add test executable
add_executable(gboviz_tests tests/test_main.cpp src/processor.cpp src/simdjson.cpp)

# Link test executable against Google Test
target_link_libraries(gboviz_tests gtest gtest_main)

# Enable testing
enable_testing()

# Add tests to CTest
add_test(NAME gboviz_tests COMMAND gboviz_tests)
