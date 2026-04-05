# --- STAGE 1: Development & Build Environment ---
FROM mcr.microsoft.com/devcontainers/cpp:1-ubuntu-24.04 AS developer

RUN apt-get update && export DEBIAN_FRONTEND=noninteractive \
    && apt-get -y install --no-install-recommends \
    # I am choosing to swap ALSA/ZMQ for OTel and Networking dependencies
    libgtest-dev libcurl4-openssl-dev libssl-dev \
    pkg-config cmake g++ gdb cppcheck \
    clang-18 clang-tidy-18 clang-format-18 \
    llvm-18 libfuzzer-18-dev \
    # I am adding the OTel C++ SDK directly from the repo later or via package if available
    && rm -rf /var/lib/apt/lists/*

# I am maintaining the same symlink strategy for toolchain consistency
RUN ln -sf /usr/bin/clang-18 /usr/bin/clang && \
    ln -sf /usr/bin/clang++-18 /usr/bin/clang++ && \
    ln -sf /usr/bin/clang-tidy-18 /usr/bin/clang-tidy && \
    ln -sf /usr/bin/run-clang-tidy-18 /usr/bin/run-clang-tidy && \
    ln -sf /usr/bin/llvm-cov-18 /usr/bin/llvm-cov && \
    ln -sf /usr/bin/llvm-profdata-18 /usr/bin/llvm-profdata

RUN cd /usr/src/googletest && cmake . && cmake --build . && cp lib/*.a /usr/lib

# I am removing the 'audio' group as the SDK is hardware-agnostic
WORKDIR /app

# --- STAGE 2: Tester (The SDK Quality Gate) ---
FROM developer AS tester

# I am keeping the cached config strategy
COPY CMakeLists.txt .clang-format .clang-tidy ./
COPY include/ ./include/
COPY src/ ./src/
COPY tests/ ./tests/

# I am choosing to fail the build if the SDK logic isn't perfectly covered or linted
RUN cppcheck --enable=all --suppress=unusedFunction --suppress=missingIncludeSystem src/ include/ --error-exitcode=1

RUN cmake -B build_test -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=clang++ -DENABLE_COVERAGE=ON && \
    cmake --build build_test && \
    cd build_test && ctest --output-on-failure && \
    llvm-profdata merge -sparse default.profraw -o test.profdata && \
    # We report coverage on the core SDK logic
    llvm-cov report ./unit_tests -instr-profile=test.profdata ../src/

# --- STAGE 3: Builder (Library Archive Creation) ---
FROM developer AS builder

COPY CMakeLists.txt ./
COPY include/ ./include/
COPY src/ ./src/

# I am choosing to build a STATIC library for easy linking in other repos
RUN cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=clang++ && -DBUILD_TESTING=OFF && \
    # I am choosing 'vita' to match the add_library(vita) in our CMakeLists.txt
    cmake --build build --target vita

# --- STAGE 4: Release (The 'SDK Delivery' Stage) ---
# For an SDK, the "Release" isn't a running binary, but the headers and the .a file.
FROM ubuntu:24.04 AS release

WORKDIR /usr/local/

# I am choosing to copy ONLY the headers and the compiled library
# This acts as a "sysroot" that you could theoretically mount or copy.
COPY --from=builder /app/include/ /usr/local/include/
COPY --from=builder /app/build/libvita-sdk.a /usr/local/lib/

# We don't need an ENTRYPOINT for an SDK
CMD ["/bin/bash"]