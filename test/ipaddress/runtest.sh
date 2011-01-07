#!/bin/sh
testname=`echo "ipaddress                        " | cut -c 1-24`
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
./ipaddress > out.txt 2>test.log || {
  echo "  failed (RUN)"
  exit 1
}
$(which echo) -n "."
echo "--- start diff" >> test.log
goodref=reference.txt
diff out.txt reference.txt >/dev/null 2>&1 || goodref=reference-alt.txt
diff out.txt $goodref >> test.log 2>&1 || {
  echo " failed (DIFF)"
  exit 1
}
goodref=reference.xml
diff out.xml $goodref >/dev/null 2>&1 || goodref=reference-alt.xml
diff out.xml $goodref >> test.log 2>&1 || {
  echo " failed (DIFF)"
  exit 1
}
rm -f test.log out.txt out.xml
echo " passed"
