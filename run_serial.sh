#!/bin/bash

#SBATCH --reservation=cpsc424
#SBATCH --cpus-per-task=20
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --time=4:00:00
#SBATCH --mem-per-cpu=5G
#SBATCH --job-name=mcts_serial
#SBATCH --output=%x-%j.out

# For OpenMP
module load intel

# Make test program
PROGRAM=mcts_connect_four
make $PROGRAM

TRIALS=5
TEST_GAMES=1000
EPSILON="0.1"
TIME_LIMIT="0.1"

echo "Running Test Script for MCTS on Connect Four"
echo ""
for (( trial=1; trial <= TRIALS; trial++ ))
do
	time "./$PROGRAM" serial random $TEST_GAMES $EPSILON $TIME_LIMIT
done
echo "**All done**"

