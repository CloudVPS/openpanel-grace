#!/bin/sh
testname=`echo "timestamp                        " | cut -c 1-24`; echo -n "${testname}: "
rm -f *.o output.xml >/dev/null 2>&1
echo -n "."
make clean >/dev/null 2>&1 || echo -n ""
echo -n "."
make > test.log 2>&1 || {
  echo "   failed (BUILD)"
  exit 1
}
echo -n "."
echo "--- start run" >> test.log
./timestamp >> test.log 2>&1 || {
  echo "  failed(RUN)"
  exit 1
}
echo -n "."
rm -f test.log
echo " passed"
