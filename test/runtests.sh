#!/bin/sh
echo "*** Building and running all test scenarios"
DIRS=`ls -1 */runtest.sh | sed -e "s/.runtest.sh$//"`
for dir in $DIRS; do
  cd $dir
  ./runtest.sh && ( make clean >/dev/null ) || true
  cd ..
done
echo "*** Quality control done"
