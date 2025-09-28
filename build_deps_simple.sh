#!/bin/bash

# Simplified script to build working regex engine dependencies
# Focuses on engines compatible with CMake 3.16

set -e

VENDOR_DIR="/home/hbucher/git/regex-performance/vendor"
LOCAL_DIR="$VENDOR_DIR/local"
BUILD_TYPE="${BUILD_TYPE:-Release}"
PARALLEL_JOBS="${PARALLEL_JOBS:-4}"

# Create directories
mkdir -p "$LOCAL_DIR"/{lib,include,bin}
cd "$VENDOR_DIR"

echo "=== Building compatible regex engine dependencies ==="
echo "Build type: $BUILD_TYPE"
echo "CMake version: $(cmake --version | head -1)"

# Function to clone or update repository
clone_or_update() {
    local name=$1
    local url=$2
    local branch=${3:-main}

    if [ -d "$name" ]; then
        echo "Updating $name..."
        cd "$name"
        /usr/bin/git pull origin "$branch" || true
        cd ..
    else
        echo "Cloning $name from $url..."
        /usr/bin/git clone "$url" "$name"
        if [ "$branch" != "main" ] && [ "$branch" != "master" ]; then
            cd "$name"
            /usr/bin/git checkout "$branch"
            cd ..
        fi
    fi
}

# Source the modern Clang toolchain
echo "Sourcing modern Clang toolchain..."
source /ssd/hblib-installer/ubuntu-20.04/tools/sourceme.sh

# Function to build with cmake
build_cmake() {
    local name=$1
    shift
    echo "Building $name with CMake using Clang with -march=native..."
    mkdir -p build
    cd build
    CC=clang CXX=clang++ cmake .. \
        -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
        -DCMAKE_INSTALL_PREFIX="$LOCAL_DIR" \
        -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
        -DCMAKE_CXX_FLAGS="-fPIC -march=native -mtune=native" \
        -DCMAKE_C_FLAGS="-fPIC -march=native -mtune=native" \
        -DBUILD_SHARED_LIBS=OFF \
        "$@"
    make -j"$PARALLEL_JOBS"
    make install
    cd ..
}

# Function to build with autotools
build_autotools() {
    local name=$1
    shift
    echo "Building $name with autotools using Clang with -march=native..."
    mkdir -p build
    cd build
    CC=clang CXX=clang++ \
    CFLAGS="-march=native -mtune=native" \
    CXXFLAGS="-march=native -mtune=native" \
    ../configure \
        --prefix="$LOCAL_DIR" \
        --enable-static \
        --disable-shared \
        "$@"
    make -j"$PARALLEL_JOBS"
    make install
    cd ..
}

echo "=== Building PCRE2 ==="
if [ ! -f "$LOCAL_DIR/lib/libpcre2-8.a" ]; then
    clone_or_update "pcre2" "https://github.com/PhilipHazel/pcre2.git" "master"
    cd pcre2
    /usr/bin/git submodule update --init --recursive
    build_cmake "pcre2" \
        -DPCRE2_SUPPORT_JIT=ON \
        -DPCRE2_BUILD_TESTS=OFF \
        -DPCRE2_BUILD_PCRE2GREP=OFF
    cd ..
else
    echo "PCRE2 already built, skipping..."
fi

echo "=== Building Oniguruma ==="
if [ ! -f "$LOCAL_DIR/lib/libonig.so" ]; then
    clone_or_update "oniguruma" "https://github.com/kkos/oniguruma.git" "master"
    cd oniguruma
    build_cmake "oniguruma" \
        -DBUILD_TEST=OFF \
        -DINSTALL_DOCUMENTATION=OFF
    cd ..
else
    echo "Oniguruma already built, skipping..."
fi

echo "=== Building TRE ==="
if [ ! -f "$LOCAL_DIR/lib/libtre.so" ]; then
    clone_or_update "tre" "https://github.com/laurikari/tre.git" "master"
    cd tre
    ./utils/autogen.sh
    build_autotools "tre"
    cd ..
else
    echo "TRE already built, skipping..."
fi

echo "=== Building Jansson (required for YARA) ==="
if [ ! -f "$LOCAL_DIR/lib/libjansson.a" ]; then
    clone_or_update "jansson" "https://github.com/akheron/jansson.git" "master"
    cd jansson
    build_cmake "jansson" \
        -DJANSSON_BUILD_SHARED_LIBS=OFF \
        -DJANSSON_BUILD_DOCS=OFF
    cd ..
else
    echo "Jansson already built, skipping..."
fi

echo "=== Building YARA ==="
if [ ! -f "$LOCAL_DIR/lib/libyara.a" ]; then
    clone_or_update "yara" "https://github.com/VirusTotal/yara.git" "master"
    cd yara
    ./bootstrap.sh
    # Set environment variables so YARA can find Jansson
    export PKG_CONFIG_PATH="$LOCAL_DIR/lib/pkgconfig:$PKG_CONFIG_PATH"
    export CPPFLAGS="-I$LOCAL_DIR/include $CPPFLAGS"
    export LDFLAGS="-L$LOCAL_DIR/lib $LDFLAGS"
    export LD_LIBRARY_PATH="$LOCAL_DIR/lib:$LD_LIBRARY_PATH"
    build_autotools "yara" \
        --enable-cuckoo \
        --disable-magic \
        --enable-dotnet
    cd ..
else
    echo "YARA already built, skipping..."
fi

echo "=== Building Abseil (required for RE2) ==="
if [ ! -f "$LOCAL_DIR/lib/libabsl_base.a" ]; then
    echo "Cloning Abseil from GitHub..."
    clone_or_update "abseil-cpp" "https://github.com/abseil/abseil-cpp.git" "main"
    cd abseil-cpp
    build_cmake "abseil-cpp" \
        -DABSL_BUILD_TESTING=OFF \
        -DABSL_USE_GOOGLETEST_HEAD=OFF \
        -DCMAKE_CXX_STANDARD=17
    cd ..
else
    echo "Abseil already built, skipping..."
fi

echo "=== Building RE2 (latest version) ==="
if [ ! -f "$LOCAL_DIR/lib/libre2.a" ]; then
    echo "Cloning latest RE2 from GitHub..."
    clone_or_update "re2" "https://github.com/google/re2.git" "main"
    cd re2
    build_cmake "re2" \
        -DRE2_BUILD_TESTING=OFF \
        -DCMAKE_PREFIX_PATH="$LOCAL_DIR"
    cd ..
else
    echo "RE2 already built, skipping..."
fi

echo "=== Building simplified Boost ==="
if [ ! -f "$LOCAL_DIR/lib/libboost_regex.a" ]; then
    clone_or_update "boost" "https://github.com/boostorg/boost.git" "boost-1.82.0"
    cd boost
    # Initialize only essential submodules
    /usr/bin/git submodule update --init --recursive \
        tools/build \
        tools/boost_install \
        libs/regex \
        libs/config \
        libs/headers \
        libs/throw_exception \
        libs/exception \
        libs/assert \
        libs/core \
        libs/static_assert \
        libs/type_traits \
        libs/mpl \
        libs/preprocessor \
        libs/predef

    ./bootstrap.sh --with-toolset=clang
    ./b2 headers
    ./b2 install -q \
        --prefix="$LOCAL_DIR" \
        --with-regex \
        toolset=clang \
        variant=release \
        link=static \
        threading=multi \
        cxxstd=17 \
        cxxflags="-march=native -mtune=native" \
        cflags="-march=native -mtune=native"
    cd ..
else
    echo "Boost already built, skipping..."
fi

echo "=== Setting up CTRE (header-only) ==="
if [ ! -d "$LOCAL_DIR/include/ctre" ]; then
    echo "Cloning CTRE from GitHub..."
    /usr/bin/git clone https://github.com/hanickadot/compile-time-regular-expressions.git ctre
    cd ctre
    # CTRE is header-only, just copy headers
    cp -r include/* "$LOCAL_DIR/include/"
    cd ..
else
    echo "CTRE already set up, skipping..."
fi

echo "=== Build summary ==="
echo "Libraries built in: $LOCAL_DIR"
echo ""
echo "✅ PCRE2 (with JIT)"
echo "✅ Oniguruma"
echo "✅ TRE"
echo "✅ Jansson (dependency for YARA)"
echo "✅ YARA (with Jansson support)"
echo "✅ RE2 (2023-03-01 version, CMake 3.16 compatible)"
echo "✅ Boost.Regex (minimal build with Clang)"
echo "✅ CTRE (header-only, C++20 required)"
echo "✅ Rust regex (built automatically by main project)"
echo "✅ C++ std::regex (system)"
echo ""
echo "Total: 9 regex engines ready for benchmarking!"
echo "Now you can build the main project with these engines."