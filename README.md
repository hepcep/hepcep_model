# Hepatitis C Elimination in PWID (HepCEP) Model

## About HepCEP
HepCEP is a validated agent-based model of the population of persons who inject drugs (PWID) in Chicago and the surrounding suburbs, Illinois, USA, including injection drug use and syringe sharing behaviors, HCV infection dynamics, HCV treatment strategies with direct acting antivirals (DAA), and treatment with medications for opioid use disorder (MOUD). HepCEP is implemented using [RepastHPC](https://repast.github.io/repast_hpc.html), an agent-based modeling system intended for large-scale distributed computing platforms and writenn in C++.

The demographic, behavioral, and social characteristics of the PWID population is generated using data from five empirical datasets on metropolitan Chicago (urban and suburban) area 127 PWID. This includes data from a large syringe service program (SSP) enrollees (n=6,000, 2006-13), the IDU data collection cycles of the National HIV Behavioral Surveillance (NHBS) survey from 2009 (n=545) and 2012 (n=209), and a social network and 130 geography study of young (ages 18-30) PWID (n=164). Data analyses from these sources is used to generate attributes for each of the estimated 32,000 PWID in the synthetic population for metropolitan Chicago in the model and includes: age, age of initiation into injection drug use, gender, race/ethnicity, zip code of residence, HCV infection status, drug sharing network degree, parameters for daily injection and syringe sharing rates, and harm reduction/syringe service program (SSP) enrollment. PWID agents may leave the population due to age-dependent death or drug use cessation and are replaced with new PWID sampled from the input data set to maintain a nearly constant population size of 32,000 for the entire course of the simulation.

Syringe sharing among PWID is modeled in HepCEP via dynamic syringe sharing networks. Network formation is determined by the probability of two PWID encountering each other in their neighborhood of residence and within the outdoor drug market areas in Chicago that attracts both urban and non-urban PWID for drug purchasing and utilization of SSPs that are also located in the same areas. Each modeled individual has an estimated number of in-network PWID partners who give syringes to the individual and out-network PWID partners who receive syringes from the individual. The network edge direction determines the flow of contaminated syringes between individuals, and thus the direction of disease transmission. The network evolves over time, and during the course of the simulation some connections (ties) may be lost, while new ties form, resulting in an approximately constant network size.

Deatils on model design, imeplementation and sources of data are available in the folowing published manuscripts:

HepCEP includes the HCV infection dynamics logic from the Agent-based Pathogen Kinetics model (APK), originally developed using [Repast Simphony](https://repast.github.io/).

>**Agent-Based Model Forecasts Aging of the Population of People Who Inject Drugs in Metropolitan Chicago and Changing Prevalence of Hepatitis C Infections**. Gutfraind A, Boodram B, Prachand N, Hailegiorgis A, Dahari H, et al. (2015). PLOS ONE 10(9): e0137993. https://doi.org/10.1371/journal.pone.0137993

Methods and results for simulating DAA treatment enrollment strategies for HCV microelimation are described in:

>**Modeling hepatitis C micro-elimination among people who inject drugs with direct-acting antivirals in metropolitan Chicago**.
Tatara E, Gutfraind A, Collier NT, Echevarria D, Cotler SJ, et al. (2022). PLOS ONE 17(3): e0264983. https://doi.org/10.1371/journal.pone.0264983

Access to treatment and medication for opioid use disorder (MOUD) for reducing opioid use and associated behavioral risk among PWID is described in:

>**Spatial inequities in access to medications for treatment of opioid use disorder highlight scarcity of 1 methadone providers under counterfactual scenarios**. Eric Tatara, Qinyun Lin, Jonathan Ozik, Marynia Kolak, Nicholson Collier, Dylan Halpern, Luc Anselin, Harel Dahari, Basmattee Boodram, John Schneider (2023). medRxiv 2023.05.12.23289915; doi: https://doi.org/10.1101/2023.05.12.23289915 



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
This work is supported by the National Institute on Drug Abuse (NIH) grant U2CDA050098 (The Methodology and Advanced Analytics Resource Center), by the National Institute of General Medical Sciences grant R01GM121600, by the National Institute of Allergy and Infectious Diseases (NIH) grant R01AI158666, by the National Institute on Drug Abuse (NIH) grant R01DA043484, and by the U.S. Department of Energy under contract number DE-AC02-06CH11357, and was completed with resources 491 provided by the Laboratory Computing Resource Center at Argonne National Laboratory (Bebop cluster). The research presented in this paper is that of the authors and does not necessarily reflect the position or policy of the National Institute on Drug Abuse or any other organization.