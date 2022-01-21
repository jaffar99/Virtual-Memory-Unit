#!/bin/bash -e
echo "Compiling and Buidling the MMU"   
make mmu     

echo "--------------- Phase 1 ---------------------"
echo "Running Phase 1"
./mmu 256 BACKING_STORE.bin addresses.txt
echo "Comparing Phase 1"
diff output256.csv correct256.csv

echo "--------------- Phase 2 ---------------------"
echo "Running Phase 2"
./mmu 128 BACKING_STORE.bin addresses.txt
echo "Comparing Phase 2"
diff output128.csv correct128.csv
