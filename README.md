# Readme for HepCEP model #

## Requirements ##

* RepastHPC and its dependencies (boost etc.). Download: https://github.com/Repast/repast.hpc/releases/download/v2.2.0/repast_hpc-2.2.0.tgz. Note that the tar ball includes a boost distribution as well as the other dependencies.
* chi_sim v0.2
  ```
  git clone git@bitbucket.org:ntcollier/chi-sim.git
  cd chi-sim
  git checkout tags/v0.2
  ```
  See the *compiling* section in `users_guide.md` for compilation instructions, and run the *install* make target.
* gtest - google's unit testing library. https://github.com/google/googletest

## Compiling ##

1. Create a directory such as  Release or Debug.
2. Copy Makefile.tmplt into the directory and rename to Makefile
3. cd into the created directory
3. Edit the Makefile for your machne.
4. Make -- targets are model, tests, and clean.

Code will compile into an X/build directory where X is the directory
created in step 1. Application binaries will also be created in X. Note that the directories `Release` and `Debug` are in .gitignore in order to keep compilation artifacts out of git.
