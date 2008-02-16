#!/bin/sh
testname=`echo "value_splice                        " | cut -c 1-24`
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
./value_splice > out.json 2>>test.log || {
  echo "  failed (RUN)"
  exit 1
}
$(which echo) -n "."
echo "--- start diff" >> test.log
diff out.json reference.json >> test.log 2>&1 || {
  echo " failed (DIFF)"
  exit 1
}
rm -f test.log out.json
echo " passed"
