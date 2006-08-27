#!/bin/sh
testname=`echo "gdbm                        " | cut -c 1-24`
echo -n "${testname}: "
rm -f *.o out.dat mydb >/dev/null 2>&1
echo -n "."
make clean >/dev/null 2>&1 || echo -n ""
echo -n "."
make > test.log 2>&1 || {
  echo "   failed (BUILD)"
  exit 1
}
echo -n "."
echo "--- start run" >> test.log
./gdbm >> test.log 2>&1 || {
  echo "  failed (RUN)"
  exit 1
}
echo -n "."
echo "--- start diff" >> test.log
diff out.dat reference.dat >> test.log 2>&1 || {
  echo " failed (DIFF)"
  exit 1
}
rm -f test.log out.dat mydb
echo " passed"