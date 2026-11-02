# Hepatitis C Elimination in PWID (HepCEP) Model

## About HepCEP
HepCEP is a validated agent-based model (ABM) of the population of persons who inject drugs (PWID) in Chicago and the surrounding suburbs, Illinois, USA, including injection drug use and syringe sharing behaviors, HCV infection dynamics, HCV treatment strategies with direct acting antivirals (DAA), and treatment with medications for opioid use disorder (MOUD). HepCEP is implemented using [RepastHPC](https://repast.github.io/repast_hpc.html), an agent-based modeling system intended for large-scale distributed computing platforms and writenn in C++.

**Schematic diagram of the Hepatitis C Elimination in PWID (HepCEP) model**<br> 
The initial synthetic model population is generated from the CNEP+ dataset and linked in a syringe-sharing network. After the model burn-in period of 365 days, the main model loop begins and each individual PWID agent executes their step behavior that in turn simulates the HCV infection dynamics. PWID who have completed a successful treatment will return to either the NAIVE or RECOVERED state depending on if they have previously recovered from an acute infection. When a PWID is cured, the model uses a CURED state but remembers past RECOVERED state or past NAIVE state and returns the PWID to the respective state after treatment. https://doi.org/10.1371/journal.pone.0264983.g001

<img src="doc/Figure%201.png" width="700">


Details on the HCV infection netowork dynamics and sources of data are described in:

>**Agent-Based Model Forecasts Aging of the Population of People Who Inject Drugs in Metropolitan Chicago and Changing Prevalence of Hepatitis C Infections**. Gutfraind A, Boodram B, Prachand N, Hailegiorgis A, Dahari H, et al. (2015). PLOS ONE 10(9): e0137993. https://doi.org/10.1371/journal.pone.0137993

Methods for simulating DAA treatment enrollment strategies for HCV microelimation are described in:

>**Modeling hepatitis C micro-elimination among people who inject drugs with direct-acting antivirals in metropolitan Chicago**.
Tatara E, Gutfraind A, Collier NT, Echevarria D, Cotler SJ, et al. (2022). PLOS ONE 17(3): e0264983. https://doi.org/10.1371/journal.pone.0264983

Access to treatment and medication for opioid use disorder (MOUD) for reducing opioid use and associated behavioral risk among PWID is described in:

>**Spatial inequities in access to medications for treatment of opioid use disorder highlight scarcity of methadone providers under counterfactual scenarios**. Eric Tatara, Qinyun Lin, Jonathan Ozik, Marynia Kolak, Nicholson Collier, Dylan Halpern, Luc Anselin, Harel Dahari, Basmattee Boodram, John Schneider (2023). medRxiv 2023.05.12.23289915; https://doi.org/10.1101/2023.05.12.23289915 

An evolutionary multi-objective optimization approach that provides a pareto-optimal set of solutions that minimizes treatment costs and HCV incidence rates is described in:

> **Multi-Objective Model Exploration of Hepatitis C Elimination in an Agent-Based Model of People who Inject Drugs**. E. Tatara et al. (2019) Winter Simulation Conference (WSC), National Harbor, MD, USA, 2019, pp. 1008-1019, https://doi.org/10.1109/WSC40007.2019.9004747.


## Requirements

The model requires a C++11 compiler, GNU make, and an MPI implementation (MPICH or OpenMPI). The following libraries are also required:

* **RepastHPC 2.3.1** — https://github.com/Repast/repast.hpc/releases/tag/v2.3.1 . The release tarball bundles Boost, NetCDF, and curl under `MANUAL_INSTALL/`.
* **chiSIM (chi_sim)** — https://github.com/Repast/chiSIM
* **GoogleTest** — https://github.com/google/googletest . Required only for the `tests` target.

## Dependency installation

`Makefile.tmplt` expects the libraries under `$HOME/sfw`. Install them with the system `mpicxx` on the execution path. The example commands below reproduce the stack used for the local/WSL build (RepastHPC 2.3.1, Boost 1.85.0, chiSIM 0.4.2, and the legacy NetCDF).

1. Boost 1.85, built with the tagged layout (which yields the `-mt-x64` library suffix):
```
cd ~/sfw
curl -LO https://archives.boost.org/release/1.85.0/source/boost_1_85_0.tar.bz2
tar xjf boost_1_85_0.tar.bz2 && cd boost_1_85_0
./bootstrap.sh --prefix=$HOME/sfw/boost-1_85_0 --with-libraries=system,filesystem,mpi,serialization,atomic
echo "using mpi : mpicxx ;" >> project-config.jam
./b2 --layout=tagged variant=release threading=multi address-model=64 -j$(nproc) stage install
```

2. NetCDF and curl, from the RepastHPC tarball's `MANUAL_INSTALL` directory (RepastHPC requires the legacy NetCDF C++ API `netcdfcpp.h`):
```
cd ~/sfw
curl -LO https://github.com/Repast/repast.hpc/releases/download/v2.3.1/repast_hpc-2.3.1.tgz
tar xzf repast_hpc-2.3.1.tgz
cd repast_hpc-2.3.1/MANUAL_INSTALL
./install.sh curl
./install.sh netcdf
```

3. RepastHPC 2.3.1, built against the Boost above using the manual-install Makefile. Building with gcc 11 or later first requires adding `#include <cfloat>` to `../src/repast_hpc/GridComponents.cpp` (for `DBL_MAX`):
```
cd ~/sfw/repast_hpc-2.3.1/MANUAL_INSTALL
make -f Makefile -j$(nproc) \
  CXX=mpicxx CXXLD=mpicxx REPAST_VERSION=2.3.1 BOOST_INFIX=-mt-x64 \
  BOOST_INCLUDE_DIR=$HOME/sfw/boost-1_85_0/include BOOST_LIB_DIR=$HOME/sfw/boost-1_85_0/lib \
  NETCDF_INCLUDE_DIR=$HOME/sfw/NetCDF/include NETCDF_LIB_DIR=$HOME/sfw/NetCDF/lib \
  NETCDF_CXX_INCLUDE_DIR=$HOME/sfw/NetCDF-cxx/include NETCDF_CXX_LIB_DIR=$HOME/sfw/NetCDF-cxx/lib \
  CURL_INCLUDE_DIR=$HOME/sfw/CURL/include CURL_LIB_DIR=$HOME/sfw/CURL/lib \
  INSTALL_DIR=$HOME/sfw/repast_hpc-2.3.1 \
  copy_includes repast_hpc
```

4. chiSIM, built against RepastHPC and Boost. Copy its `Makefile.tmplt` into a build directory, set the RepastHPC and Boost paths and `VERSION=0.4.2`, then install:
```
cd ~/sfw
git clone https://github.com/Repast/chiSIM.git chiSIM-src
cd chiSIM-src && mkdir -p Release && cp Makefile.tmplt Release/Makefile
# edit Release/Makefile: REPAST_* -> $HOME/sfw/repast_hpc-2.3.1, BOOST -> $HOME/sfw/boost-1_85_0
#   (-mt-x64 libs), VERSION=0.4.2, INSTALL_DIR=$HOME/sfw/chiSIM-0.4.2
cd Release && make install
```
`make install` copies the library and headers to `$HOME/sfw/chiSIM-0.4.2`.

## Compiling the model

1. Copy `Makefile.tmplt` into a build directory (`Release` or `Debug`) as `Makefile`.
2. Edit the dependency paths at the top of the Makefile for the machine.
3. `cd` into the build directory and run `make`.

Targets are `model` (the standalone executable), `tests` (GoogleTest unit tests), and `clean`. Objects compile into `<dir>/build`; binaries are written to `<dir>`. `Release` and `Debug` are in `.gitignore`.

A ready-to-use `Release/Makefile` for a WSL build (MPICH, dependencies under `$HOME/sfw`) is included. From the `Release` directory, `make` builds the `hepcep_model-0.0` executable.

## Running the model

```
./Release/hepcep_model-0.0 -props ./config/model.props -config ./config/config.props -params "data.dir=./data"
```

`-props` names the model parameter file, `-config` the logging configuration, and `-params` a tab-separated list of `key=value` overrides parsed by `hepcep::parse_parameters`. The `data.dir` parameter sets the input-data directory; it is not defined in `model.props` by default and must be supplied on the command line or added to `model.props`. `runhepcep.sh` wraps the base command.

Output is written to the directory named by `output.directory` in `model.props` (`output` by default). Under MPI the model is distributed across ranks, for example `mpirun -n 4 ./Release/hepcep_model-0.0 -props ./config/model.props -config ./config/config.props -params "data.dir=./data"`.

## Acknowledgements
This work is supported by the National Institute on Drug Abuse (NIH) grant U2CDA050098 (The Methodology and Advanced Analytics Resource Center), by the National Institute of General Medical Sciences grant R01GM121600, by the National Institute of Allergy and Infectious Diseases (NIH) grant R01AI158666, by the National Institute on Drug Abuse (NIH) grant R01DA043484, and by the U.S. Department of Energy under contract number DE-AC02-06CH11357, and was completed with resources provided by the Laboratory Computing Resource Center at Argonne National Laboratory (Bebop cluster). The research presented in this paper is that of the authors and does not necessarily reflect the position or policy of the National Institute on Drug Abuse or any other organization.