#!/bin/sh

# This file is part of the Grace library (libgrace).
# The Grace library is free software: you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License as published by the
# Free Software Foundation, using version 3 of the License.
# You should have received a copy of the GNU Lesser General Public License 
# along with Grace library. If not, see <http://www.gnu.org/licenses/>.

if [ -z "$1" ]; then
  echo "% Usage: $0 <testname>"
  exit 1
fi

TESTNAME="$1"
if [ -d "$TESTNAME" ]; then
  echo "% '$TESTNAME' already exists"
  exit 1
fi

mkdir "$TESTNAME"
mkdir "$TESTNAME"/rsrc
for file in Makefile runtest.sh main.cpp; do
  sed -e "s/%TESTNAME%/${TESTNAME}/g" < _template/${file}.in > "$TESTNAME"/$file
done
chmod 755 "$TESTNAME"/runtest.sh
