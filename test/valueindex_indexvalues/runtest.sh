#!/bin/sh
testname=`echo "valueindex_indexvalues                        " | cut -c 1-24`
echo -n "${testname}: "
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
./valueindex_indexvalues >> test.log 2>&1 || {
  echo "  failed (RUN)"
  exit 1
}
./valueindex_indexvalues > out.dat
echo -n "."
echo "--- start diff" >> test.log
diff out.dat reference.dat >> test.log 2>&1 || {
  echo " failed (DIFF)"
  exit 1
}
rm -f test.log
echo " passed"
