#!/bin/sh
testname=`echo "pcre                        " | cut -c 1-24`
$(which echo) -n "${testname}: "
rm -f *.o output.xml >/dev/null 2>&1
$(which echo) -n "."
./configure > test.log 2>&1 || {
  echo "   failed (CONFIGURE)"
  exit 1
}
make clean >/dev/null 2>&1 || $(which echo) -n ""
$(which echo) -n "."
make >> test.log 2>&1 || {
  echo "   failed (BUILD)"
  exit 1
}
$(which echo) -n "."
echo "--- start run" >> test.log
./pcre >> test.log 2>&1 || {
  echo "  failed (RUN)"
  exit 1
}
$(which echo) -n "."
#echo "--- start diff" >> test.log
#diff out.dat reference.dat >> test.log 2>&1 || {
#  echo " failed (DIFF)"
#  exit 1
#}
rm -f test.log
echo " passed"
