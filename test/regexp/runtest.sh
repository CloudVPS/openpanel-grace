#!/bin/sh
echo -n "regexp: "
rm -f *.o output.xml >/dev/null 2>&1
make clean >/dev/null 2>&1 || echo -n ""
make > test.log 2>&1 || {
  echo "failed (BUILD)"
  exit 1
}
echo "--- start run" >> test.log
./regexp >> test.log 2>&1 || {
  echo "failed (RUN)"
  exit 1
}
rm -f test.log
echo "passed"
