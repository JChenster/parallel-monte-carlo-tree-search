#!/bin/bash

#SBATCH --reservation=cpsc424
#SBATCH --cpus-per-task=20
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --time=4:00:00
#SBATCH --mem-per-cpu=4G
#SBATCH --job-name=mcts_tgm_parallel
#SBATCH --output=%x-%j.out

# For OpenMP
module load intel

# Make test program
PROGRAM=mcts_connect_four
make $PROGRAM

TRIALS=3
TEST_GAMES=1000
EPSILON="0.1"
TIME_LIMIT="0.1"

echo "Running Test Script for MCTS on Connect Four"
for THREADS in 1 2 4 10 20
do
	export OMP_NUM_THREADS=$THREADS
	echo "Threads: $THREADS"
	echo ""
	for (( trial=1; trial <= TRIALS; trial++ ))
	do
		echo "Trial $trial"
		time "./$PROGRAM" tgm random $TEST_GAMES $EPSILON $TIME_LIMIT
		echo ""
	done
done
echo "**All done**"

