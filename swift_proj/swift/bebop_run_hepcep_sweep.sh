#! /usr/bin/env bash
set -eu

if [ "$#" -ne 2 ]; then
  script_name=$(basename $0)
  echo "Usage: ${script_name} EXPERIMENT_ID CONFIG_FILE(e.g. ${script_name} experiment_1 model.props)"
  exit 1
fi

# uncomment to turn on swift/t logging. Can also set TURBINE_LOG,
# TURBINE_DEBUG, and ADLB_DEBUG to 0 to turn off logging
# export TURBINE_LOG=1 TURBINE_DEBUG=1 ADLB_DEBUG=1
export EMEWS_PROJECT_ROOT=$( cd $( dirname $0 )/.. ; /bin/pwd )
# source some utility functions used by EMEWS in this script
source "${EMEWS_PROJECT_ROOT}/etc/emews_utils.sh"

export EXPID=$1
export TURBINE_OUTPUT=$EMEWS_PROJECT_ROOT/experiments/$EXPID
check_directory_exists

# TODO edit the number of processes as required.
# 288
export PROCS=216

# TODO edit QUEUE, WALLTIME, PPN, AND TURNBINE_JOBNAME
# as required. Note that QUEUE, WALLTIME, PPN, AND TURNBINE_JOBNAME will
# be ignored if the MACHINE variable (see below) is not set.
#export QUEUE=bdwall
#export PROJECT=emews
export PROJECT=g-DIS
export QUEUE=dis
export WALLTIME=12:00:00
export PPN=18
export TURBINE_JOBNAME="${EXPID}_job"

export PYTHONPATH=$EMEWS_PROJECT_ROOT/python:$EMEWS_PROJECT_ROOT/ext/EQ-Py

# TODO edit command line arguments as appropriate
# for your run. Note that the default $* will pass all of this script's
# command line arguments to the swift script.
CMD_LINE_ARGS="$*"

CONFIG_FILE=$EMEWS_PROJECT_ROOT/../config/$2
MODEL_DIR=$EMEWS_PROJECT_ROOT/model

# Root folder for all input files. e.g. population CSVs
DATA_DIR=$EMEWS_PROJECT_ROOT/../data

#export TURBINE_LAUNCHER=srun

# set machine to your schedule type (e.g. pbs, slurm, cobalt etc.),
# or empty for an immediate non-queued unscheduled run
MACHINE="pbs"

if [ -n "$MACHINE" ]; then
  MACHINE="-m $MACHINE"
fi

READLINE_LIB=/gpfs/fs1/soft/bebop/software/spack-built/linux-rocky8-x86_64/gcc-8.5.0/readline-8.2-chmbmda/lib
MKL="/gpfs/fs1/soft/bebop/software/spack-built/linux-rocky8-x86_64/oneapi-2024.1.0/intel-oneapi-mkl-2024.0.0-2ug2cu4/mkl/2024.0/lib"
BOOST_LIB="/lcrc/project/EMEWS/bebop-2.0/sfw/gcc-11.4.0/openmpi-4.1.1/boost-1_85_0/lib"
export LD_LIBRARY_PATH="$READLINE_LIB:$MKL:$BOOST_LIB:$LD_LIBRARY_PATH"

export LD_PRELOAD="/gpfs/fs1/soft/bebop/software/custom-built/openmpi/4.1.1/gcc/8.5.0/lib/libmpi.so"

export TURBINE_LAUNCH_OPTIONS="--"

# Add any script variables that you want to log as
# part of the experiment meta data to the USER_VARS array,
# for example, USER_VARS=("VAR_1" "VAR_2")
USER_VARS=()
# log variables and script to to TURBINE_OUTPUT directory
log_script

# echo's anything following this standard out
set -x

swift-t -n $PROCS $MACHINE -p -r $MODEL_DIR -I $MODEL_DIR \
  $EMEWS_PROJECT_ROOT/swift/hepcep_sweep.swift \
  -f="$EMEWS_PROJECT_ROOT/data/upf_enrollment_sweep_screen_APK_VK.txt" \
  -config_file=$CONFIG_FILE \
  -data_dir=$DATA_DIR \
  $CMD_LINE_ARGS
