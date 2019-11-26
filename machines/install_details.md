# MIDAS Install Details #

Everything but hepcep installed in `/home/hepcep/sfw`. Environment setup with:

```
module load gcc/9.1.0
module load openmpi/4.0.1
module load python/anaconda3.7-2019.03
module load r/3.6.1_no-MPI

. /home/hepcep/sfw/spack/share/spack/setup-env.sh
```

## Repast HPC dependencies ##
* curl - installed via install.sh curl (see `/home/hepcep/nick_collier_midasnetwork_us/src/repast_hpc-2.3.0/MANUAL_INSTALL/install.sh`)
* netcdf / netcdf-cxx - installed via install.sh netcdf (see `/home/hepcep/nick_collier_midasnetwork_us/src/repast_hpc-2.3.0/MANUAL_INSTALL/install.sh`)
* boost - installed via spack because the version included with repast hpc was incompatible with Midas' openmpi version (`spack install boost %gcc@9.1.0 +mpi +taggedlayout cxxstd=11`)

## Repast HPC ##

```
BOOST_DIR=/home/hepcep/sfw/spack/opt/spack/linux-centos7-haswell/gcc-9.1.0/boost-1.70.0-3ucptyn2pvvjacqqmkkupax6pmokqdcg
BASE_DIR=/home/hepcep/sfw
CXX="mpicxx" CXXLD="mpicxx" ./configure --prefix=/home/hepcep/sfw/repast_hpc-2.3.0 --with-boost-include=$BOOST_DIR/include --with-boost-lib-dir=$BOOST_DIR/lib     --with-boost-lib-suffix=x64     --with-netcdf-cxx=$BASE_DIR/netcdf-cxx --with-netcdf=$BASE_DIR/netcdf --with-curl-include=$BASE_DIR/curl/include --with-curl-lib-dir=$BASE_DIR/curl/lib
```

## chiSIM ##

Using v0.2. 

```
git clone git@bitbucket.org:ntcollier/chi-sim.git
git fetch --all --tags --prune
git checkout tags/v0.2 -b v0.2
make install
```
Using the makefile in `/home/hepcep/nick_collier_midasnetwork_us/repos/chi-sim/Release/Makefile`

## HepCep ##

Cloned from repo and made with Makefile in 

`/home/hepcep/nick_collier_midasnetwork_us/repos/hepcep_model/Release/Makefile`

# Bebop Details #

... 