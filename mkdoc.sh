#!/bin/sh
doxygen doxygenconf 2>&1 | egrep -v "^libgd was not built with|^ : Helvetica"
cp doc/stylesheet/doxygen.css doc/generated/html/
