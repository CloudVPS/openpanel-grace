#!/bin/sh
testname=`echo "querido                        " | cut -c 1-24`
$(which echo) -n "${testname}: "
rm -f *.o out.xml row.xml out.sql db.sqlite >/dev/null 2>&1
sqlite3 db.sqlite < db.sqlite.in
$(which echo) -n "."
make clean >/dev/null 2>&1 || $(which echo) -n ""
$(which echo) -n "."
make > test.log 2>&1 || {
  echo "   failed (BUILD)"
  exit 1
}
$(which echo) -n "."
echo "--- start run" >> test.log
./querido >> test.log 2>&1 || {
  echo "  failed (RUN)"
  exit 1
}
$(which echo) -n "."
echo "--- start diff" >> test.log
diff out.sql reference.sql >> test.log 2>&1 || {
  echo " failed (DIFF)"
  exit 1
}
diff out.xml reference.xml >> test.log 2>&1 || {
  echo " failed (DIFF)"
  exit 1
}
diff row.xml row.reference.xml >> test.log 2>&1 || {
  echo " failed (DIFF)"
  exit 1
}
rm -f test.log
echo " passed"
