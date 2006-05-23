#!/bin/sh
if [ ! -d matrixobj ]; then
  mkdir matrixobj
else
  rm -f matrixobj/*.o
fi
cd matrixobj
ar x ../../matrixssl/src/libmatrixsslstatic.a
cd ..

