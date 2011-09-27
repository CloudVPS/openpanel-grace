#!/bin/sh
testname=`echo "strgrow                        " | cut -c 1-24`
$(which echo) -n "${testname}: "
rm -f *.o s1.out s2.out >/dev/null 2>&1
$(which echo) -n "."
make clean >/dev/null 2>&1 || $(which echo) -n ""
$(which echo) -n "."
make > test.log 2>&1 || {
  echo "   failed (BUILD)"
  exit 1
}
$(which echo) -n "."
echo "--- start run" >> test.log
./strgrow >> test.log 2>&1 || {
  echo "  failed (RUN)"
  exit 1
}
$(which echo) -n "."
echo "--- start diff" >> test.log
diff s1.out s1.reference >> test.log 2>&1 || {
  echo " failed (DIFF)"
  exit 1
}
diff s2.out s2.reference >> test.log 2>&1 || {
  echo " failed (DIFF)"
  exit 1
}
rm -f test.log
echo " passed"
