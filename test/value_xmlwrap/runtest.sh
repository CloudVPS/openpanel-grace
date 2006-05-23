#!/bin/sh
echo -n "value_xmlwrap: "
rm -f *.o out.xml out.csv >/dev/null 2>&1
make clean >/dev/null 2>&1 || echo -n ""
make > test.log 2>&1 || {
  echo "failed (BUILD)"
  exit 1
}
echo "--- start run" >> test.log
./value_xmlwrap >> test.log 2>&1 || {
  echo "failed (RUN)"
  exit 1
}
echo "--- start diff" >> test.log
diff out.xml reference.xml >> test.log 2>&1 || {
  echo "failed (DIFF)"
  exit 1
}
diff out2.xml reference2.xml >> test.log 2>&1 || {
  echo "failed (DIFF)"
  exit 1
}
rm -f test.log out.xml out2.xml
echo "passed"
