#!/bin/sh
testname=`echo "value_plist                        " | cut -c 1-24`; echo -n "${testname}: "
rm -f *.o out.xml out.plist >/dev/null 2>&1
echo -n "."
make clean >/dev/null 2>&1 || echo -n ""
echo -n "."
make > test.log 2>&1 || {
  echo "   failed (BUILD)"
  exit 1
}
echo -n "."
echo "--- start run" >> test.log
./value_plist >> test.log 2>&1 || {
  echo "  failed(RUN)"
  exit 1
}
echo -n "."
echo "--- start diff" >> test.log
diff out.xml reference.xml >> test.log 2>&1 || {
  echo " failed (DIFF)"
  exit 1
}
diff out.plist reference.plist >> test.log 2>&1 || {
  echo " failed (DIFF)"
  exit 1
}
rm -f test.log out.plist out.xml
echo " passed"
