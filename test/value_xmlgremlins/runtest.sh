#!/bin/sh
echo -n "value_xmlgremlins: "
rm -f *.o output.xml >/dev/null 2>&1
make clean >/dev/null 2>&1 || echo -n ""
make > test.log 2>&1 || {
  echo "failed (BUILD)"
  exit 1
}
echo "--- start run" >> test.log
./value_xmlgremlins >> test.log 2>&1 || {
  echo "failed (RUN)"
  exit 1
}
echo "--- start diff" >> test.log
diff out.xml reference.xml >/dev/null 2>&1 || {
  echo "failed (DIFF)"
  exit 1
}
rm -f test.log out.xml
echo "passed"
