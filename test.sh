#!/bin/bash

#SBATCH --reservation=cpsc424
#SBATCH --cpus-per-task=20
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --time=4:00:00
#SBATCH --mem-per-cpu=2G
#SBATCH --job-name=test
#SBATCH --output=%x-%j.out

# For omp
module load intel

# Make test program

g++ -fopenmp -o test test.cpp

export OMP_NUM_THREADS=4
./test

