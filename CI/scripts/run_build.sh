#!/bin/bash

set -e

sudo apt-get update

TOP_DIR="$(pwd)"
DEPS_DIR="${TOP_DIR}/deps"

. ./CI/scripts/lib.sh

build_astyle() {
    . ./CI/scripts/astyle.sh
}

build_cppcheck() {
    . ./CI/scripts/cppcheck.sh
}

build_${BUILD_TYPE}
