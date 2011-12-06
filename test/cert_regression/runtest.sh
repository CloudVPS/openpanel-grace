#!/bin/sh
testname=`echo "cert_regression                       " | cut -c 1-24`; $(which echo) -n "${testname}: "

rm -f *.o default.* localhost.* >/dev/null 2>&1
$(which echo) -n "."

make clean >/dev/null 2>&1 || $(which echo) -n ""
$(which echo) -n "."

make > test.log 2>&1 || {
  echo "   failed (BUILD)"
  exit 1
}

$(which echo) -n "."
echo "--- start run" >> test.log
./https >> test.log 2>&1 || {
  echo "  failed(RUN)"
  exit 1
}

$(which echo) -n "."
echo "--- start diff" >> test.log

diff localhost reference.localhost >> test.log 2>&1 || {
  echo " failed (DIFF)"
  exit 1
}

rm -rf test.log localhost https.app https https.exe
echo " passed"
