%module hepcep_model

%include <std_string.i>

%include "../src/run.h"


%{
  #include "mpi.h"
  #include "../src/run.h"

  typedef struct ompi_communicator_t* MPI_Comm;
%}

typedef struct ompi_communicator_t* MPI_Comm;
