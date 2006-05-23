#!/bin/sh
echo -n "value_ini: "
rm -f *.o output.xml >/dev/null 2>&1
make > test.log 2>&1 || {
  echo "failed (BUILD)"
  exit 1
}
echo "--- start run" >> test.log
./value_ini >> test.log 2>&1 || {
  echo "failed (RUN)"
  exit 1
}
echo "--- start diff" >> test.log
diff output.xml reference.xml >> test.log 2>&1 || {
  echo "failed (DIFF)"
  exit 1
}
rm -f test.log output.xml
echo "passed"
