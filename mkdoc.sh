#!/bin/sh

# This file is part of the Grace library (libgrace).
# The Grace library is free software: you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License as published by the
# Free Software Foundation, either version 3 of the License.
# You should have received a copy of the GNU Lesser General Public License 
# along with Grace library. If not, see <http://www.gnu.org/licenses/>.

doxygen doxygenconf 2>&1 | egrep -v "^libgd was not built with|^ : Helvetica"
#cp doc/stylesheet/doxygen.css doc/generated/html/
