cmake_minimum_required(VERSION 3.24)

# I am choosing to let the toolchain handle the heavy lifting.
if(EXISTS "/opt/vcpkg/scripts/buildsystems/vcpkg.cmake")
    set(CMAKE_TOOLCHAIN_FILE "/opt/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

project(vita-sdk VERSION 0.1.0 LANGUAGES CXX)

# --- 0. TOOLCHAIN HARDENING ---
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(
        -Wall -Wextra -Wpedantic -Wshadow -Wnon-virtual-dtor
        -Wold-style-cast -Wcast-align -Wunused -Woverloaded-virtual
        -Wconversion -Wsign-conversion -Wnull-dereference
        -Wdouble-promotion -Wformat=2
    )
endif()

# --- 1. COVERAGE CONFIGURATION ---
option(ENABLE_COVERAGE "Enable code coverage reporting" OFF)
if(ENABLE_COVERAGE)
    add_compile_options(-fprofile-instr-generate -fcoverage-mapping -fno-inline -g -O0)
    add_link_options(-fprofile-instr-generate -fcoverage-mapping)
endif()

# --- 2. FIND PACKAGES & AUDIT ---
find_package(Threads REQUIRED)
find_package(CURL REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(opentelemetry-cpp REQUIRED)
# I am choosing to provide a specific hint for httplib since the 
# standard vcpkg search is failing for this specific package.
set(httplib_DIR "/opt/vcpkg/installed/x64-linux/share/httplib")
# I am choosing to require cpp-httplib for our health monitoring server.
# I am choosing to find httplib using its actual exported CMake name.
# The audit revealed that even if the vcpkg port is 'cpp-httplib', 
# the configuration file is named 'httplibConfig.cmake'.
find_package(httplib CONFIG REQUIRED)

message(STATUS "--- VITA SDK FILESYSTEM AUDIT ---")

# Audit the OTel Resource Header location
find_path(OTEL_INCLUDE_PATH
    NAMES opentelemetry/sdk/resource/resource.h
    PATHS /opt/vcpkg/installed/x64-linux/include
    NO_DEFAULT_PATH
)

if(OTEL_INCLUDE_PATH)
    message(STATUS "[SUCCESS] OTel Headers found at: ${OTEL_INCLUDE_PATH}")
else()
    message(WARNING "[AUDIT FAIL] OTel Headers NOT found in expected vcpkg path!")
endif()

# Audit the httplib location discovered in the manual search
find_path(HTTPLIB_INCLUDE_PATH
    NAMES httplib.h
    PATHS /opt/vcpkg/installed/x64-linux/include
    NO_DEFAULT_PATH
)

if(HTTPLIB_INCLUDE_PATH)
    message(STATUS "[SUCCESS] httplib Header found at: ${HTTPLIB_INCLUDE_PATH}")
else()
    message(WARNING "[AUDIT FAIL] httplib Header NOT found!")
endif()

# Audit the Semantic Conventions (Using the advisory bypass)
find_file(OTEL_SEMCONV_HEADER
    NAMES semantic_conventions.h resource_attributes.h
    PATH_SUFFIXES sdk/resource semconv
    PATHS ${OTEL_INCLUDE_PATH}/opentelemetry
)

if(NOT OTEL_SEMCONV_HEADER)
    message(WARNING "[ADVISORY] Semantic Conventions header missing. Using internal string literal fallback.")
endif()

message(STATUS "--- END VITA SDK AUDIT ---")

option(BUILD_TESTING "Build the testing tree." ON)
if(BUILD_TESTING)
    find_package(GTest REQUIRED)
endif()

# --- 3. THE SDK LIBRARY TARGET ---
add_library(vita STATIC)

target_include_directories(vita PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# I am choosing to add health.cpp and trace.cpp to the library sources.
target_sources(vita 
    PRIVATE 
        src/config.cpp
        src/telemetry.cpp
        src/health.cpp
        # src/trace.cpp
)

# I am choosing to link cpp-httplib and the verified OTel targets.
target_link_libraries(vita 
    PUBLIC 
        opentelemetry-cpp::resources
        opentelemetry-cpp::trace 
        opentelemetry-cpp::common
        opentelemetry-cpp::sdk
        opentelemetry-cpp::otlp_http_exporter
        httplib::httplib
        Threads::Threads
        CURL::libcurl
        OpenSSL::SSL
)

# --- 4. GOOGLETEST (Validation) ---
if(BUILD_TESTING)
    enable_testing()
    
    add_executable(unit_tests 
        tests/unit/test_config.cpp 
        tests/unit/test_telemetry.cpp 
        # tests/unit/test_health.cpp # I am choosing to prepare for health tests.
    )

    target_link_libraries(unit_tests 
        PRIVATE 
            vita 
            opentelemetry-cpp::resources
            opentelemetry-cpp::common
            opentelemetry-cpp::trace
            GTest::gtest 
            GTest::gtest_main
    )

    include(GoogleTest)
    gtest_discover_tests(unit_tests)
endif()

# --- 5. INSTALLATION ---
install(TARGETS vita DESTINATION lib)
install(DIRECTORY include/vita DESTINATION include)