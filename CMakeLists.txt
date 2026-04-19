cmake_minimum_required(VERSION 3.24)

# I am choosing to let the toolchain handle the heavy lifting.
if(EXISTS "/opt/vcpkg/scripts/buildsystems/vcpkg.cmake")
    set(CMAKE_TOOLCHAIN_FILE "/opt/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

project(vita-sdk VERSION 0.1.0 LANGUAGES CXX)

# --- 0. TOOLCHAIN & ENVIRONMENT HARDENING ---
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# I am choosing to suppress the VCPKG_ROOT mismatch warning observed in the audit.
set(VCPKG_ROOT "/opt/vcpkg" CACHE PATH "")

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

# --- 1. THE MANUAL OVERRIDE (Audit Solution) ---
# I am choosing to manually locate the Brotli headers and libraries 
# because the vcpkg Config files are failing to manifest.
find_path(BROTLI_INCLUDE_DIR NAMES brotli/decode.h PATHS /opt/vcpkg/installed/x64-linux/include)
find_library(BROTLI_COMMON_LIB NAMES brotlicommon PATHS /opt/vcpkg/installed/x64-linux/lib)
find_library(BROTLI_ENC_LIB NAMES brotlienc PATHS /opt/vcpkg/installed/x64-linux/lib)
find_library(BROTLI_DEC_LIB NAMES brotlidec PATHS /opt/vcpkg/installed/x64-linux/lib)

# I am choosing to create the unofficial namespace httplib expects.
if(BROTLI_INCLUDE_DIR AND BROTLI_COMMON_LIB)
    add_library(unofficial::brotli::brotlicommon UNKNOWN IMPORTED)
    set_target_properties(unofficial::brotli::brotlicommon PROPERTIES
        IMPORTED_LOCATION "${BROTLI_COMMON_LIB}"
        INTERFACE_INCLUDE_DIRECTORIES "${BROTLI_INCLUDE_DIR}")

    add_library(unofficial::brotli::brotlienc UNKNOWN IMPORTED)
    set_target_properties(unofficial::brotli::brotlienc PROPERTIES
        IMPORTED_LOCATION "${BROTLI_ENC_LIB}")

    add_library(unofficial::brotli::brotlidec UNKNOWN IMPORTED)
    set_target_properties(unofficial::brotli::brotlidec PROPERTIES
        IMPORTED_LOCATION "${BROTLI_DEC_LIB}")
    
    set(unofficial-brotli_FOUND TRUE CACHE BOOL "" FORCE)
    message(STATUS "[SUCCESS] Brotli manually mapped from ${BROTLI_INCLUDE_DIR}")
endif()

# --- 2. FIND PACKAGES & AUDIT RESOLUTION ---
find_package(Threads REQUIRED)
find_package(CURL REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(opentelemetry-cpp REQUIRED)

# Now httplib should load because we've provided its missing 'unofficial-brotli' dependency manually.
find_package(httplib CONFIG REQUIRED HINTS "/opt/vcpkg/installed/x64-linux/share/httplib")

message(STATUS "--- VITA SDK FILESYSTEM AUDIT ---")
find_path(OTEL_INCLUDE_PATH
    NAMES opentelemetry/sdk/resource/resource.h
    PATHS /opt/vcpkg/installed/x64-linux/include
    NO_DEFAULT_PATH
)

if(OTEL_INCLUDE_PATH)
    message(STATUS "[SUCCESS] OTel Headers found at: ${OTEL_INCLUDE_PATH}")
else()
    message(WARNING "[AUDIT FAIL] OTel Headers NOT found!")
endif()
message(STATUS "--- END VITA SDK AUDIT ---")

option(BUILD_TESTING "Build the testing tree." ON)
if(BUILD_TESTING)
    find_package(GTest REQUIRED)
endif()

# --- 3. THE SDK LIBRARY TARGET ---
# I am choosing to keep this as a SHARED library for better OTel provider visibility.
add_library(vita SHARED)

target_include_directories(vita PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# I am choosing to include the newly written trace.cpp and health logic.
target_sources(vita 
    PRIVATE 
        src/config.cpp
        src/telemetry.cpp
        src/health.cpp
        src/log.cpp
        src/trace.cpp
        src/metric.cpp
)

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
        tests/unit/test_health.cpp 
        tests/unit/log_test.cpp 
        tests/unit/trace_test.cpp
        tests/unit/metric_test.cpp
    )

    target_link_libraries(unit_tests 
        PRIVATE 
            vita 
            httplib::httplib
            GTest::gtest 
            GTest::gtest_main
    )

    include(GoogleTest)
    gtest_discover_tests(unit_tests)
endif()

# --- 5. INSTALLATION ---
install(TARGETS vita DESTINATION lib)
install(DIRECTORY include/vita DESTINATION include)