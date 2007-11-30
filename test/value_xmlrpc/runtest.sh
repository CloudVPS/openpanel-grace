#!/bin/sh
testname=`echo "value_xmlrpc                        " | cut -c 1-24`; $(which echo) -n "${testname}: "
rm -f *.o out.xml out.csv >/dev/null 2>&1
$(which echo) -n "."
make clean >/dev/null 2>&1 || $(which echo) -n ""
$(which echo) -n "."
make > test.log 2>&1 || {
  echo "   failed (BUILD)"
  exit 1
}
$(which echo) -n "."
echo "--- start run" >> test.log
./value_xmlrpc >> test.log 2>&1 || {
  echo "  failed(RUN)"
  exit 1
}
$(which echo) -n "."
echo "--- start diff" >> test.log
diff out.xml reference.xml >> test.log 2>&1 || {
  echo " failed (DIFF)"
  exit 1
}
diff out.xmlrpc reference.xmlrpc >> test.log 2>&1 || {
  echo " failed (DIFF)"
  exit 1
}
rm -f test.log out.xml out.xmlrpc
echo " passed"
