#!/bin/sh
testname=`echo "http                        " | cut -c 1-24`; echo -n "${testname}: "
rm -f *.o default.* localhost.* >/dev/null 2>&1
echo -n "."
make clean >/dev/null 2>&1 || echo -n ""
echo -n "."
make > test.log 2>&1 || {
  echo "   failed (BUILD)"
  exit 1
}
echo -n "."
echo "--- start run" >> test.log
./http >> test.log 2>&1 || {
  echo "  failed(RUN)"
  exit 1
}
echo -n "."
echo "--- start diff" >> test.log
for file in default.public default.restricted localhost.public localhost.restricted; do
  diff $file reference.$file >> test.log 2>&1 || {
    echo " failed (DIFF)"
    exit 1
  }
done
rm -rf test.log default.public default.restricted localhost.public localhost.restricted http.app http
echo " passed"
