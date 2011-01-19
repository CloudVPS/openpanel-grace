#!/bin/sh
testname=`echo "nonewline                        " | cut -c 1-24`; $(which echo) -n "${testname}: "
rm -f *.o output.xml >/dev/null 2>&1
$(which echo) -n "."
make clean >/dev/null 2>&1 || $(which echo) -n ""
$(which echo) -n "."
make > test.log 2>&1 || {
  echo "   failed (BUILD)"
  exit 1
}
$(which echo) -n "."
echo "--- start run" >> test.log
printf abc | ./nonewline >> test.log 2>&1 || {
  echo "  failed(RUN)"
  exit 1
}
$(which echo) -n "."
rm -f test.log
echo " passed"
