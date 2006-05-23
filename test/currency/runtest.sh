#!/bin/sh
echo -n "currency: "
rm -f *.o output.xml >/dev/null 2>&1
make clean >/dev/null 2>&1 || echo -n ""
make > test.log 2>&1 || {
  echo "failed (BUILD)"
  exit 1
}
echo "--- start run" >> test.log
./currency >> test.log 2>&1 || {
  echo "failed (RUN)"
  exit 1
}
echo "--- start diff" >> test.log
diff out.xml reference.xml >/dev/null 2>&1 || {
  echo "failed (DIFF)"
  exit 1
}
diff out2.xml reference2.xml >/dev/null 2>&1 || {
  echo "failed (DIFF)"
  exit 1
}
diff out3.xml reference.xml >/dev/null 2>&1 || {
  echo "failed (DIFF)"
  exit 1
}
rm -f test.log out.xml out2.xml
echo "passed"
