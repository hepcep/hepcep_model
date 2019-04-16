#! /usr/bin/env bash

set -eu

if [ "$#" -ne 2 ]; then
  script_name=$(basename $0)
  echo "Usage: ${script_name} EXPERIMENT_ID CONFIG_FILE(e.g. ${script_name} experiment_1 model.props)"
  exit 1
fi

# Emews with Python+R
#PATH=/lcrc/project/EMEWS/bebop/sfw/swift-t-12202018/stc/bin/:$PATH

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
export PROCS=36

# TODO edit QUEUE, WALLTIME, PPN, AND TURNBINE_JOBNAME
# as required. Note that QUEUE, WALLTIME, PPN, AND TURNBINE_JOBNAME will
# be ignored if MACHINE flag (see below) is not set
export QUEUE=bdwall
export WALLTIME=00:20:00
export PPN=36
export TURBINE_JOBNAME="${EXPID}_job"

# if R cannot be found, then these will need to be
# uncommented and set correctly.
# export R_HOME=/path/to/R
# export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$R_HOME/lib
#export PYTHONHOME=/soft/anaconda3/5.2.0/lib/python3.6
export PYTHONPATH=$EMEWS_PROJECT_ROOT/python:$EMEWS_PROJECT_ROOT/ext/EQ-Py
#export PYTHONPATH=$EMEWS_PROJECT_ROOT/python:$EMEWS_PROJECT_ROOT/ext/EQ-Py:/soft/anaconda3/5.2.0/lib/python3.6/site-packages:/soft/anaconda3/5.2.0/lib/python3.6:/soft/anaconda3/5.2.0/lib/python3.6/lib-dynload

# Resident task workers and ranks
export TURBINE_RESIDENT_WORK_WORKERS=1
export RESIDENT_WORK_RANKS=$(( PROCS - 2 ))

# EQ/Py location
EQPY=$EMEWS_PROJECT_ROOT/ext/EQ-Py

# TODO edit command line arguments, e.g. -nv etc., as appropriate
# for your EQ/Py based run. $* will pass all of this script's
# command line arguments to the swift script
SEED=1234
ITERS=2
NUM_VARIATIONS=2
NUM_POP=4

TISD=0.25


# original was 0.2
MUTATION_PROB=0.2

CONFIG_FILE=$EMEWS_PROJECT_ROOT/../config/$2
MODEL_DIR=$EMEWS_PROJECT_ROOT/model

# set machine to your schedule type (e.g. pbs, slurm, cobalt etc.),
# or empty for an immediate non-queued unscheduled run
MACHINE="slurm"

if [ -n "$MACHINE" ]; then
  MACHINE="-m $MACHINE"
fi

# Add any script variables that you want to log as
# part of the experiment meta data to the USER_VARS array,
# for example, USER_VARS=("VAR_1" "VAR_2")
USER_VARS=()
# log variables and script to to TURBINE_OUTPUT directory
log_script

# echo's anything following this to standard out

set -x
SWIFT_FILE=ga_workflow.swift
swift-t -n $PROCS $MACHINE -p -I $EQPY -I $MODEL_DIR -r $EQPY -r $MODEL_DIR \
    $EMEWS_PROJECT_ROOT/swift/$SWIFT_FILE \
    -ni=$ITERS \
    -nv=$NUM_VARIATIONS \
    -np=$NUM_POP \
	-seed=$SEED \
    -mutation_prob=$MUTATION_PROB \
	-ga_params="$EMEWS_PROJECT_ROOT/data/ga_params.json" \
	-config_file=$CONFIG_FILE