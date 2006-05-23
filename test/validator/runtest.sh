#!/bin/sh
echo -n "validator: "
rm -f *.o >/dev/null 2>&1
make clean >/dev/null 2>&1 || echo -n ""
make > test.log 2>&1 || {
  echo "failed (BUILD)"
  exit 1
}
echo "--- start run" >> test.log
./validator >> test.log 2>&1 || {
  echo "failed (RUN)"
  exit 1
}
./validator > out
echo "--- start diff" >> test.log
diff out reference >> test.log 2>&1 || {
  echo "failed (DIFF)"
  exit 1
}
rm -f test.log out
echo "passed"
