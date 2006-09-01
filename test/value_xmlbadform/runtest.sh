#!/bin/sh
testname=`echo "value_xmlbadform                        " | cut -c 1-24`
echo -n "${testname}: "
rm -f *.o out.txt >/dev/null 2>&1
echo -n "."
make clean >/dev/null 2>&1 || echo -n ""
echo -n "."
make > test.log 2>&1 || {
  echo "   failed (BUILD)"
  exit 1
}
echo -n "."
echo "--- start run" >> test.log
./value_xmlbadform >> test.log 2>&1 || {
  echo "  failed (RUN)"
  exit 1
}
echo -n "."
echo "--- start diff" >> test.log
diff out.txt reference.txt >> test.log 2>&1 || {
  echo " failed (DIFF)"
  exit 1
}
rm -f test.log out.txt
echo " passed"
