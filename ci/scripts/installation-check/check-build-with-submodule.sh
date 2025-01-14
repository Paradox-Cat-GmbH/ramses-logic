#!/bin/bash

#  -------------------------------------------------------------------------
#  Copyright (C) 2020 BMW AG
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------

if [ $# -eq 3 ]; then
    TEST_DIR=$1
    SRC_DIR=$2
    RAMSES_INSTALL=$3
else
    echo "Usage: $0 <temp-build-dir> <src-dir> <ramses-install-dir>"
    exit 1
fi

set -e

SCRIPT_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

echo "++++ Create test environment for building statically ++++"

rm -rf $TEST_DIR

# Create temporary build directory
mkdir -p $TEST_DIR/ramses-logic
cd $TEST_DIR
# Base project setup
cp -r $SCRIPT_DIR/submodule-check/* .
# Add ramses logic as submodule to folder ramses-logic
cp -r $SRC_DIR/* ./ramses-logic/

echo "++++ building and executing test binary ++++"

# Build with cmake
mkdir build
cd build
cmake -GNinja -DCMAKE_PREFIX_PATH=$RAMSES_INSTALL ../
cmake --build . --target run-all

echo "++++ build check done for static submodule build ++++"
