#!/bin/bash
# Builds, in build/, the example etatest (examples/etatest.cc) and the Catch2 unit tests etadecayer_tests
# (test/testEtaDecayer.cc).
# Needs: a C++17 compiler (CXX, default g++), pythia8-config and root-config in PATH, and for the unit tests Catch2 v3
# installed under CATCH2_DIR (include/catch2/catch_all.hpp and lib or lib64/libCatch2Main, libCatch2); the default
# CATCH2_DIR is the installation prefix of pythia8-config, which is right for an LCG view. Without Catch2 only
# etatest is built.
set -e
cd "$(dirname "$0")"
for tool in pythia8-config root-config; do
  command -v $tool >/dev/null || { echo "$tool not found in PATH"; exit 1; }
done
CXX=${CXX:-g++}
FLAGS="-O2 -std=c++17 -Wall -Iinclude $(pythia8-config --cxxflags) $(root-config --cflags)"
LIBS="$(pythia8-config --libs) $(root-config --libs)"
SRC=src/EtaDecayer.cc
mkdir -p build

$CXX $FLAGS examples/etatest.cc $SRC -o build/etatest $LIBS
echo "built $(pwd)/build/etatest (Pythia $(pythia8-config --version), ROOT $(root-config --version))"

CATCH2_DIR=${CATCH2_DIR:-$(dirname "$(dirname "$(command -v pythia8-config)")")}
CATCH2_LIB=""
for d in lib64 lib; do
  if [ -f "$CATCH2_DIR/$d/libCatch2Main.a" ] || [ -f "$CATCH2_DIR/$d/libCatch2Main.so" ]; then
    CATCH2_LIB=$CATCH2_DIR/$d
    break
  fi
done
if [ -f "$CATCH2_DIR/include/catch2/catch_all.hpp" ] && [ -n "$CATCH2_LIB" ]; then
  $CXX $FLAGS -I"$CATCH2_DIR/include" test/testEtaDecayer.cc $SRC -o build/etadecayer_tests $LIBS \
    -L"$CATCH2_LIB" -lCatch2Main -lCatch2
  echo "built $(pwd)/build/etadecayer_tests"
else
  echo "Catch2 v3 not found under CATCH2_DIR=$CATCH2_DIR: unit tests not built"
fi
