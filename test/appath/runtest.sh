#!/bin/sh
testname=`echo "appath                        " | cut -c 1-24`
echo -n "${testname}: "
rm -f *.o output.xml extrasoftlink >/dev/null 2>&1
echo -n "."
make clean >/dev/null 2>&1 || echo -n ""
echo -n "."
make > test.log 2>&1 || {
  echo "   failed (BUILD)"
  exit 1
}
echo -n "."
echo "--- start run" >> test.log
./extrasoftlink >> test.log 2>&1 || {
  echo "  failed (RUN)"
  exit 1
}
echo -n "."
#echo "--- start diff" >> test.log
#diff out.dat reference.dat >> test.log 2>&1 || {
#  echo " failed (DIFF)"
#  exit 1
#}
rm -f test.log extrasoftlink
echo " passed"
