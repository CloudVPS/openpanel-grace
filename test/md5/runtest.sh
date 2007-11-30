#!/bin/sh
testname=`echo "md5                        " | cut -c 1-24`
$(which echo) -n "${testname}: "
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
./md5 >> test.log 2>&1 || {
  echo "  failed (RUN)"
  exit 1
}
$(which echo) -n "."
echo "--- start diff" >> test.log
diff out1.md5 reference1.md5 >> test.log 2>&1 || {
  echo " failed (DIFF)"
  exit 1
}
diff out2.md5 reference2.md5 >> test.log 2>&1 || {
  echo " failed (DIFF)"
  exit 1
}
rm -f test.log out1.md5 out2.md5
echo " passed"
