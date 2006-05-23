#!/bin/sh
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
