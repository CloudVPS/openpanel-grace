#!/bin/sh
echo -n "strutil1: "
rm -f *.o out.dat out.xml wrapped.txt >/dev/null 2>&1
make clean >/dev/null 2>&1 || echo -n ""
make > test.log 2>&1 || {
  echo "failed (BUILD)"
  exit 1
}
echo "--- start run" >> test.log
./strutil1 >> test.log 2>&1 || {
  echo "failed (RUN)"
  exit 1
}
echo "--- start diff" >> test.log
diff out.xml reference.xml >> test.log 2>&1 || {
  echo "failed (DIFF)"
  exit 1
}
diff out.dat reference.dat >> test.log 2>&1 || {
  echo "failed (DIFF)"
  exit 1
}
diff wrapped.txt reference.txt >> test.log 2>&1 || {
  echo "failed (DIFF)"
  exit 1
}
rm -f test.log out.dat out.xml wrapped.txt
echo "passed"
