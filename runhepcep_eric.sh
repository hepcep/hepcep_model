#!/bin/bash
#
# Run the chi-sim for Eric's system

export LD_LIBRARY_PATH=/mnt/c/repast_hpc-2.2.0/ext/lib:/mnt/c/HepCEP/chiSIM-0.2/lib
mpirun -n 1 ./Debug/hepcep_model-0.0 ./config/model.props

