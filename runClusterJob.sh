#!/usr/bin/env bash

COUNT=${1}
TIMELIMIT=${2}
TESTSET=${3}
RESULTS=${4}


module purge
module load compilers/gcc-10.2.0

for ((i=1; i<=COUNT; i++))
do
    INSTANCE=inputFile${i}
    
    bsub -J ${INSTANCE}_s \
         -n 1 \
         -W ${TIMELIMIT} \
         -q batch \
         -R "span[ptile=36]" \
         -e ${RESULTS}/err/testFile${i}_s.err \
         -o ${RESULTS}/out/testFile${i}_s.out \
         ./BestFirstSearch_s ${TESTSET}/${INSTANCE}.csv ${RESULTS}/txt/outputFile${i}_s.txt
    
    bsub -J ${INSTANCE}_p \
         -n 36 \
         -W ${TIMELIMIT} \
         -q batch \
         -R "span[ptile=36]" \
         -e ${RESULTS}/err/testFile${i}_p.err \
         -o ${RESULTS}/out/testFile${i}_p.out \
         ./BestFirstSearch_p ${TESTSET}/${INSTANCE}.csv ${RESULTS}/txt/outputFile${i}_p.txt
done
