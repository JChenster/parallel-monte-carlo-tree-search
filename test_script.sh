#!/bin/bash

#SBATCH --reservation=cpsc424
#SBATCH --cpus-per-task=20
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --time=4:00:00
#SBATCH --mem-per-cpu=2G
#SBATCH --job-name=mcts_lp
#SBATCH --output=%x-%j.out

# For omp
module load intel

# Make test program
PROGRAM=mcts_connect_four
make clean
make $PROGRAM

THREADS=4
TRIALS=5

OMP_NUM_THREADS=$THREADS

echo "Running Test Script for MCTS on Connect Four"
echo ""
for (( trial=1; trial <= TRIALS; trial++ ))
do
	time "./$PROGRAM"
done
echo "**All done**"

