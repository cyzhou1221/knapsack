#!/usr/bin/env bash

COUNT=${1}
RESULTS=${2}

cd ${RESULTS}/out
rm -f testFile.out

for ((i=1; i<=COUNT; i++))
do
   cat testFile${i}_s.out >> testFile.out;
   cat testFile${i}_p.out >> testFile.out;
done

cd ../../

awk -f resultsCompare.awk ${RESULTS}/out/testFile.out
