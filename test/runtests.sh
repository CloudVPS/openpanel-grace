#!/bin/sh
echo "*** Building and running all test scenarios"
DIRS=`ls -1 */runtest.sh | sed -e "s/.runtest.sh$//"`
errors=0
for dir in $DIRS; do
  cd $dir
  ./runtest.sh && ( make clean >/dev/null ) || errors=$((errors + 1))
  if [ -e test.log ]; then cat test.log; fi
  cd ..
done
if [ "$errors" = "0" ]; then
  echo "*** Quality control done, no errors. Time to ship!"
else
  if [ "$errors" = "1" ]; then
    echo "*** Quality control done, 1 error."
  else
    echo "*** Quality control done, $errors errors."
  fi
fi