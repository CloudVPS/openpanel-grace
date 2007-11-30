#!/bin/sh
testname=`echo "value_stresstest                        " | cut -c 1-24`; $(which echo) -n "${testname}: "
rm -f *.o >/dev/null 2>&1
$(which echo) -n "."
make clean >/dev/null 2>&1 || $(which echo) -n ""
$(which echo) -n "."
make > test.log 2>&1 || {
  echo "   failed (BUILD)"
  exit 1
}
$(which echo) -n "."
echo "--- start run" >> test.log
./value_stresstest >> test.log 2>&1 || {
  echo "  failed(RUN)"
  exit 1
}
$(which echo) -n "."
rm -f test.log stringrefs.dat
echo " passed"
