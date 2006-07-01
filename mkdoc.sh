#!/bin/sh
doxygen doxygenconf 2>&1 | egrep "^/"
cp doc/stylesheet/doxygen.css doc/generated/html/
