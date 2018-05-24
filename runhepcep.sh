#!/bin/bash
#
# Run HepCEP model using default model and config props

# Uncomment the LD_LIBRARY_PATH in case not defined in environment already.
#export LD_LIBRARY_PATH=/mnt/e/repast_hpc-2.2.0/ext/lib:/mnt/e/ANL/Projects/HepCEP/chiSIM-0.2/lib
mpirun -n 1 ./Release/hepcep_model-0.0 ./config/model.props ./config/config.props
