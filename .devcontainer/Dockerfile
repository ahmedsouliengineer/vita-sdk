# --- STAGE 1: The Pre-built Development Environment ---
# I am choosing to start from the GHCR image to skip the OTel build time.
FROM ghcr.io/ahmedsouliengineer/vita-base:latest AS developer

# The devcontainer.json uses this stage.
# We don't need to COPY code here because VS Code mounts your workspace automatically.
WORKDIR /app

# --- STAGE 2: Tester (For CI/CD) ---
FROM developer AS tester
COPY CMakeLists.txt .clang-format .clang-tidy ./
COPY include/ ./include/
COPY src/ ./src/
COPY tests/ ./tests/

# I am choosing to run tests during the image build to catch regressions early.
RUN cmake -B build_test \
    -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_CXX_COMPILER=clang++ \
    -DENABLE_COVERAGE=ON && \
    cmake --build build_test && \
    cd build_test && ctest --output-on-failure

# --- STAGE 3: Builder (Production Library) ---
FROM developer AS builder
COPY CMakeLists.txt ./
COPY include/ ./include/
COPY src/ ./src/

RUN cmake -B build \
    -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_COMPILER=clang++ \
    -DBUILD_TESTING=OFF && \
    cmake --build build --target vita

# --- STAGE 4: Final Release (Minimal SDK) ---
FROM ubuntu:24.04 AS release
WORKDIR /usr/local/

# I am choosing to copy ONLY the necessary artifacts for an end-user of the SDK.
COPY --from=builder /app/include/ /usr/local/include/
COPY --from=builder /app/build/libvita.a /usr/local/lib/

# I am choosing to also export the OTel headers/libs from vcpkg 
# so users of 'libvita.a' can resolve the OTel symbols.
COPY --from=builder /opt/vcpkg/installed/x64-linux/lib/ /usr/local/lib/
COPY --from=builder /opt/vcpkg/installed/x64-linux/include/ /usr/local/include/

CMD ["/bin/bash"]



# # --- STAGE 4: Final Release (Minimal SDK) ---
# FROM ubuntu:24.04 AS release
# WORKDIR /usr/local/

# # Copy your built library
# COPY --from=builder /app/include/ /usr/local/include/
# COPY --from=builder /app/build/libvita.a /usr/local/lib/

# # I am choosing to use a glob to copy only the .a and .so files from vcpkg
# # This keeps the final SDK image clean of build-only metadata.
# COPY --from=builder /opt/vcpkg/installed/x64-linux/lib/*.a /usr/local/lib/
# COPY --from=builder /opt/vcpkg/installed/x64-linux/lib/*.so* /usr/local/lib/
# COPY --from=builder /opt/vcpkg/installed/x64-linux/include/ /usr/local/include/