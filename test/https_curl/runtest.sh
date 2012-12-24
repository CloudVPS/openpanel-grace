#!/bin/sh
testname=`echo "https_curl                             " | cut -c 1-24`; 
$(which echo) -n "${testname}: "
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
./https >> test.log 2>&1 &

# DISABLE TLS, as it isn't supported by the GPL version of MatrixSSL
#curl -k1 -m1 -otls1.txt https://localhost:14265/test.txt || {
#    echo " failed (RUN1)"
#    wait
#    exit 1
#}

sleep 1

curl -k3 -m1 -ossl3.txt https://localhost:14265/test.txt >>test.log 2>&1 || {
    echo " failed (CURL)"
    wait
    exit 1
}
#curl -k2 -m1 -ossl2.txt https://localhost:14265/test.txt 2>/dev/null || {
#    echo " failed (RUN)"
#    wait
#    exit 1
#}
wait

$(which echo) -n "."
echo "--- start diff" >> test.log
for file in ssl3.txt ssl2.txt tls1.txt; do
  if [ -f "$file" ]; then
    diff $file docroot/test.txt >> test.log 2>&1 || {
      echo " failed (DIFF)"
      exit 1
    }
  fi
done
rm -rf ssl3.txt ssl2.txt http test.log
echo " passed"
