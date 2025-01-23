#!/usr/bin/env bash

BINFILE=${1} 
OUT=${2}     
TEST=ctestlib
RESULT=ctestResult
COUNT=10

# generate results & compare them with correct solutions
mkdir ${OUT}
for ((j=1; j<=COUNT; j++))
do
  ./${BINFILE} ${TEST}/input$j.csv ${OUT}/output$j.txt
done

for ((j=1; j<=COUNT; j++))
do
  diff -s ${TEST}/output$j.txt ${OUT}/output$j.txt >> ${OUT}/${RESULT} || true # ignore return value: 0 success, 1 same, 2 error
done
