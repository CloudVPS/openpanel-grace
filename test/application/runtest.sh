#!/bin/sh
echo -n "application: "
rm -f *.o output.xml >/dev/null 2>&1
make clean >/dev/null 2>&1 || echo -n ""
make > test.log 2>&1 || {
  echo "failed (BUILD)"
  exit 1
}
echo "--- start run" >> test.log
./testscr.sh >> test.log 2>&1 || {
  echo "failed (RUN)"
  exit 1
}
echo "--- start diff" >> test.log
diff out.dat reference.dat >> test.log 2>&1 || {
  echo "failed (DIFF)"
  exit 1
}

rm -f test.log out.dat
echo "passed"
