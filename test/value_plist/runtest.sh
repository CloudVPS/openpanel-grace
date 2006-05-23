#!/bin/sh
echo -n "value_plist: "
rm -f *.o out.xml out.plist >/dev/null 2>&1
make > test.log 2>&1 || {
  echo "failed (BUILD)"
  exit 1
}
echo "--- start run" >> test.log
./value_plist >> test.log 2>&1 || {
  echo "failed (RUN)"
  exit 1
}
echo "--- start diff" >> test.log
diff out.xml reference.xml >> test.log 2>&1 || {
  echo "failed (DIFF)"
  exit 1
}
diff out.plist reference.plist >> test.log 2>&1 || {
  echo "failed (DIFF)"
  exit 1
}
rm -f test.log out.plist out.xml
echo "passed"
