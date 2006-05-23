#!/bin/sh
echo -n "value_csv: "
rm -f *.o out.xml out.csv >/dev/null 2>&1
make > test.log 2>&1 || {
  echo "failed (BUILD)"
  exit 1
}
echo "--- start run" >> test.log
./value_csv >> test.log 2>&1 || {
  echo "failed (RUN)"
  exit 1
}
echo "--- start diff" >> test.log
diff out.xml reference.xml >> test.log 2>&1 || {
  echo "failed (DIFF)"
  exit 1
}
diff out.csv reference.csv >> test.log 2>&1 || {
  echo "failed (DIFF)"
  exit 1
}
rm -f test.log out.csv out.xml
echo "passed"
