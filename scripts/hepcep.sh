#!/bin/bash

set -eu

# Check for an optional timeout threshold in seconds. If the duration of the
# model run as executed below, takes longer that this threshhold
# then the run will be aborted. Note that the "timeout" command
# must be supported by executing OS.

# The timeout argument is optional. By default the "run_model" swift
# app fuction sends 3 arguments, and no timeout value is set. If there
# is a 4th (the TIMEOUT_ARG_INDEX) argument, we use that as the timeout value.

# !!! IF YOU CHANGE THE NUMBER OF ARGUMENTS PASSED TO THIS SCRIPT, YOU MUST
# CHANGE THE TIMEOUT_ARG_INDEX !!!
TIMEOUT=""
TIMEOUT_ARG_INDEX=4
if [[ $# ==  $TIMEOUT_ARG_INDEX ]]
then
	TIMEOUT=${!TIMEOUT_ARG_INDEX}
fi

TIMEOUT_CMD=""
if [ -n "$TIMEOUT" ]; then
  TIMEOUT_CMD="timeout $TIMEOUT"
fi

# Set param_line from the first argument to this script
# param_line is the string containing the model parameters for a run.
param_line=$1

# Set emews_root to the root directory of the project (i.e. the directory
# that contains the scripts, swift, etc. directories and files)
emews_root=$2

# Each model run, runs in its own "instance" directory
# Set instance_directory to that and cd into it.
instance_directory=$3
cd $instance_directory

# TODO: Define the command to run the model. For example,
# MODEL_CMD="python"
MODEL_CMD=""
# TODO: Define the arguments to the MODEL_CMD. Each rguments should be
# surrounded by quotes and separated by spaces. For example,
# arg_array=("$emews_root/python/nt3_tc1_runner.py" "$parameter_string")
#arg_array=("arg1" "arg2" "arg3")
COMMAND="mpirun -n 1 $emews_root/Debug/hepcep_model-0.0 $emews_root/config/emews_model.props $emews_root/config/config.props $param_line"

# Turn bash error checking off. This is
# required to properly handle the model execution return value
# the optional timeout.
set +e
echo "Running $COMMAND"

$TIMEOUT_CMD $COMMAND
# $? is the exit status of the most recently executed command (i.e the
# line above)
RES=$?
if [ "$RES" -ne 0 ]; then
	if [ "$RES" == 124 ]; then
    echo "---> Timeout error in $COMMAND"
  else
	   echo "---> Error in $COMMAND"
  fi
fi
