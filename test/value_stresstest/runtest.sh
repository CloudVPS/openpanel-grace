#!/bin/sh
echo -n "value_stresstest: "
rm -f *.o >/dev/null 2>&1
make > test.log 2>&1 || {
  echo "failed (BUILD)"
  exit 1
}
echo "--- start run" >> test.log
./value_stresstest >> test.log 2>&1 || {
  echo "failed (RUN)"
  exit 1
}
rm -f test.log stringrefs.dat
echo "passed"
