#!/bin/sh
testname=`echo "value_xmlescape                        " | cut -c 1-24`
$(which echo) -n "${testname}: "
rm -f *.o out.xml >/dev/null 2>&1
$(which echo) -n "."
make clean >/dev/null 2>&1 || $(which echo) -n ""
$(which echo) -n "."
make > test.log 2>&1 || {
  echo "   failed (BUILD)"
  exit 1
}
$(which echo) -n "."
echo "--- start run" >> test.log
./value_xmlescape >> test.log 2>&1 || {
  echo "  failed (RUN)"
  exit 1
}
$(which echo) -n "."
echo "--- start diff" >> test.log
diff out.xml reference.xml >> test.log 2>&1 || {
  echo " failed (DIFF)"
  exit 1
}
rm -f test.log out.xml
echo " passed"
