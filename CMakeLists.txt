cmake_minimum_required(VERSION 3.24)

# I am naming the project vita-sdk to reflect its purpose as a foundational toolkit.
project(vita-sdk VERSION 0.1.0 LANGUAGES CXX)

# --- 0. TOOLCHAIN HARDENING ---
# I am forcing C++23 to ensure concepts and std::same_as are recognized globally.
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# I am enabling this so the linter (clangd) can read the flags used during build.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Replace your current Clang block with this:
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(
        -Wall -Wextra -Wpedantic 
        --gcc-install-dir=/usr/lib/gcc/x86_64-linux-gnu/13
    )
    # I am choosing to remove experimental flags to let C++23 features run natively.
endif()

# --- 1. FIND PACKAGES ---
find_package(GTest REQUIRED)
find_package(Threads REQUIRED)

# OpenTelemetry dependencies
find_package(CURL REQUIRED)
find_package(OpenSSL REQUIRED)

# --- 2. THE SDK LIBRARY TARGET ---
# I am choosing to keep it STATIC for easier linking into microservices.
add_library(vita STATIC)

target_include_directories(vita PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# I am choosing to only include config.cpp for now as requested.
target_sources(vita PRIVATE 
    src/config.cpp
)

target_link_libraries(vita 
    PUBLIC 
        Threads::Threads
        CURL::libcurl
        OpenSSL::SSL
)

# --- 3. GOOGLETEST (Validation) ---
enable_testing()
# I am choosing to create a dedicated test runner for the configuration logic.
add_executable(unit_tests 
    tests/unit/test_config.cpp 
)

target_link_libraries(unit_tests 
    PRIVATE 
        vita 
        GTest::gtest 
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(unit_tests)

# --- 4. INSTALLATION ---
install(TARGETS vita DESTINATION lib)
install(DIRECTORY include/vita DESTINATION include)