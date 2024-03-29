#!/bin/bash

#SBATCH --reservation=cpsc424
#SBATCH --cpus-per-task=20
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --time=4:00:00
#SBATCH --mem-per-cpu=4G
#SBATCH --job-name=mcts_tnm_parallel
#SBATCH --output=%x-%j.out

# For OpenMP
module load intel
# Set up OpenMP
THREADS=2
OMP_NUM_THREADS=$THREADS

# Make test program
PROGRAM=mcts_connect_four
make $PROGRAM

TRIALS=10
TEST_GAMES=1
EPSILON="0.15"
TIME_LIMIT="0.01"

echo "Running Test Script for MCTS on Connect Four"
echo "Threads: $THREADS"
echo ""
for (( trial=1; trial <= TRIALS; trial++ ))
do
	time "./$PROGRAM" tnm random $TEST_GAMES $EPSILON $TIME_LIMIT
done
echo "**All done**"

