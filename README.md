# Hepatitis C Elimination in PWID (HepCEP) Model

## About HepCEP
HepCEP is a validated agent-based model (ABM) of the population of persons who inject drugs (PWID) in Chicago and the surrounding suburbs, Illinois, USA, including injection drug use and syringe sharing behaviors, HCV infection dynamics, HCV treatment strategies with direct acting antivirals (DAA), and treatment with medications for opioid use disorder (MOUD). HepCEP is implemented using [RepastHPC](https://repast.github.io/repast_hpc.html), an agent-based modeling system intended for large-scale distributed computing platforms and writenn in C++.

**Schematic diagram of the Hepatitis C Elimination in PWID (HepCEP) model**<br> 
The initial synthetic model population is generated from the CNEP+ dataset and linked in a syringe-sharing network. After the model burn-in period of 365 days, the main model loop begins and each individual PWID agent executes their step behavior that in turn simulates the HCV infection dynamics. PWID who have completed a successful treatment will return to either the NAIVE or RECOVERED state depending on if they have previously recovered from an acute infection. When a PWID is cured, the model uses a CURED state but remembers past RECOVERED state or past NAIVE state and returns the PWID to the respective state after treatment.

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

* RepastHPC and its dependencies (boost etc.). Download: https://github.com/Repast/repast.hpc/releases/download/v2.2.0/repast_hpc-2.2.0.tgz. Note that the tar ball includes a boost distribution as well as the other dependencies.
* chi_sim v0.2
  ```
  git clone git@bitbucket.org:ntcollier/chi-sim.git
  cd chi-sim
  git checkout tags/v0.2
  ```
  See the *compiling* section in `users_guide.md` for compilation instructions, and run the *install* make target.
* gtest - google's unit testing library. https://github.com/google/googletest

## Compiling

1. Create a directory such as  Release or Debug.
2. Copy Makefile.tmplt into the directory and rename to Makefile
3. cd into the created directory
3. Edit the Makefile for your machne.
4. Make -- targets are model, tests, and clean.

Code will compile into an X/build directory where X is the directory
created in step 1. Application binaries will also be created in X. Note that the directories `Release` and `Debug` are in .gitignore in order to keep compilation artifacts out of git.

## Acknowledgements
This work is supported by the National Institute on Drug Abuse (NIH) grant U2CDA050098 (The Methodology and Advanced Analytics Resource Center), by the National Institute of General Medical Sciences grant R01GM121600, by the National Institute of Allergy and Infectious Diseases (NIH) grant R01AI158666, by the National Institute on Drug Abuse (NIH) grant R01DA043484, and by the U.S. Department of Energy under contract number DE-AC02-06CH11357, and was completed with resources provided by the Laboratory Computing Resource Center at Argonne National Laboratory (Bebop cluster). The research presented in this paper is that of the authors and does not necessarily reflect the position or policy of the National Institute on Drug Abuse or any other organization.