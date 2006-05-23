#!/bin/sh
echo -n "value_sort: "
rm -f *.o natural.xml record.xml >/dev/null 2>&1
make > test.log 2>&1 || {
  echo "failed (BUILD)"
  exit 1
}
echo "--- start run" >> test.log
./value_sort >> test.log 2>&1 || {
  echo "failed (RUN)"
  exit 1
}
echo "--- start diff" >> test.log
diff natural.xml natural.reference.xml >> test.log 2>&1 || {
  echo "failed (DIFF)"
  exit 1
}
diff record.xml record.reference.xml >> test.log 2>&1 || {
  echo "failed (DIFF)"
  exit 1
}
rm -f test.log natural.xml record.xml
echo "passed"
