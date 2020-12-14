#!/bin/bash
set -e -x

# To be executed inside manylinux docker container

REPOPATH="$( cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
echo "Path of repository is: ${REPOPATH}"

PYTHONS=()
for PYBIN in /opt/python/*/bin; do
    for BLOCKED in $PYTHON_BLOCKLIST; do
        if [[ $PYBIN == *"$BLOCKED"* ]]; then
            PYBIN=""
            break
        fi
    done
    PYTHONS+=( $PYBIN )
done

echo "Used Python versions:"
for PYBIN in "${PYTHONS[@]}"; do
    echo "${PYBIN} at version:"
    "${PYBIN}/python" --version
done
echo

# manylinux 2014 images now have cmake installed
if ! command -v /usr/local/bin/cmake &> /dev/null; then
    if [ "$(arch)" = "i686" ] || [ "$(arch)" = "x86_64" ]; then
        echo "Installing cmake from pypi..."
        PYBIN="${PYTHONS[0]}"
        "${PYBIN}/pip" install cmake
        ln -s "${PYBIN}/cmake" /usr/local/bin/cmake
    elif [ "$(arch)" = "aarch64" ] || [ "$(arch)" = "ppc64le" ]; then
        # Cmake not available or incompatible from pypi for this platform
        echo "Installing cmake from epel..."
        yum install -y epel-release
        yum install -y cmake3
        ln -s /usr/bin/cmake3 /usr/local/bin/cmake
    else
        CMAKE_VERSION=3.18.2
        # Build current cmake, epel is either outdated or to old
        echo "Building cmake..."
        yum install -y wget
        wget -qO /root/cmake.tar.gz https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/cmake-${CMAKE_VERSION}.tar.gz
        mkdir /root/cmake-src
        cd /root/cmake-src
        tar -xzf /root/cmake.tar.gz --strip-components 1 -C ./
        ./bootstrap -- -DCMAKE_BUILD_TYPE:STRING=Release -DCMAKE_USE_OPENSSL=OFF
        make
        make install
        cd /
    fi
fi
cmake --version

function repair_wheel {
    wheel="$1"
    if ! auditwheel show "$wheel"; then
        echo "Skipping non-platform wheel $wheel"
    else
        auditwheel repair "$wheel" --plat "$PLAT" -w "${WHEELS_TESTED}"
    fi
}


# Install a system package required by our library
yum install -y              \
    libXrandr-devel         \
    libXinerama-devel       \
    libXcursor-devel        \
    libXi-devel

WHEELS_BUILD="$(mktemp -d)"
WHEELS_TESTED="$(mktemp -d)"

# Compile wheels
for PYBIN in "${PYTHONS[@]}"; do
    "${PYBIN}/pip" install -r "${REPOPATH}/dev-requirements.txt"
    "${PYBIN}/pip" wheel "${REPOPATH}/" --no-deps --no-build-isolation -w "${WHEELS_BUILD}"
done

# Bundle external shared libraries into the wheels
for whl in "${WHEELS_BUILD}"/*.whl; do
    repair_wheel "$whl"
done

# Install packages and test
for PYBIN in "${PYTHONS[@]}"; do
    "${PYBIN}/pip" install mahi-gui --no-index -f "${WHEELS_TESTED}"
    "${PYBIN}/nosetests" -w "${REPOPATH}/tests"
done

# Publish wheels only if no errors occured:
mkdir -p "${REPOPATH}/wheelhouse"
mv "${WHEELS_TESTED}"/*.whl "${REPOPATH}/wheelhouse"

rm -rf "${WHEELS_BUILD}"
rm -rf "${WHEELS_TESTED}"
