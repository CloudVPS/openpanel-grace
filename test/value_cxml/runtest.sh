#!/bin/sh
testname=`echo "value_cxml                        " | cut -c 1-24`; echo -n "${testname}: "
rm -f *.o output.xml >/dev/null 2>&1
echo -n "."
make clean >/dev/null 2>&1 || echo -n ""
echo -n "."
make > test.log 2>&1 || {
  echo "   failed (BUILD)"
  exit 1
}
echo -n "."
echo "--- start run" >> test.log
./value_cxml >> test.log 2>&1 || {
  echo "  failed(RUN)"
  exit 1
}
echo -n "."
echo "--- start diff" >> test.log
diff out.cxml reference.cxml >> test.log 2>&1 || {
  echo " failed (DIFF)"
  exit 1
}
diff out.xml reference.xml >> test.log 2>&1 || {
  echo " failed (DIFF)"
  exit 1
}
rm -f test.log out.cxml out.xml
echo " passed"
