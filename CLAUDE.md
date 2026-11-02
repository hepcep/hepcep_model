# HepCEP (C++ / RepastHPC)

## Overview
HepCEP is an agent-based model of persons who inject drugs (PWID) in metropolitan
Chicago. It simulates syringe-sharing network dynamics, HCV infection, DAA
treatment, and medications for opioid use disorder (MOUD). This repository is the
original, validated implementation written in C++ on
[RepastHPC](https://repast.github.io/repast_hpc.html) and the `chi_sim` layer, and
runs distributed across MPI ranks.

A newer Python/Repast4Py port lives in the sibling directory `../hepcep4py`. See
[Relationship to the Python port](#relationship-to-the-python-port) for the feature
differences. Note that `hepcep4py` contains a Python package also named
`hepcep_model`, which is unrelated to this C++ source tree beyond modeling the same
system.

## Repository layout
- `src/` — all C++ sources and headers (the `module.mk` `SRC` list is the
  authoritative set of compiled units).
- `config/` — `model.props` (model parameters) and `config.props` (logging).
- `data/`, `test_data/` — CNEP+ PWID catalog, zone files, viral-kinetics CSVs.
- `test/` — GoogleTest unit tests (`hc_tests.cpp`, `network_tests.cpp`, `test_main.cpp`).
- `Makefile.tmplt` — template Makefile; copy into a build dir and edit per machine.
- `swift_proj/`, `machines/`, `experiments/` — Swift/T EMEWS sweep workflows and
  cluster (Bebop) launch support.
- `scripts/` — R and R-markdown analysis of model output (not part of the model build).
- `grammar/`, `src/gml*.c`, `src/lex.yy.c` — flex/bison GML network-file parser.

## Build
RepastHPC 2.2.0 (bundles boost and other deps), `chi_sim` v0.4, and GoogleTest are
required; see `README.md` for dependency install.

1. Create a build directory (`Release` or `Debug`; both are gitignored).
2. Copy `Makefile.tmplt` into it as `Makefile` and edit paths for the machine.
3. `cd` into the directory and run `make`. Targets: `model`, `tests`, `clean`.

Objects compile into `<dir>/build/`; binaries land in `<dir>/`.

## Run
```
./Release/hepcep_model-0.0 -props ./config/model.props -config ./config/config.props
```
`runhepcep.sh` wraps this. Optional `-params "<tab-separated overrides>"` overrides
individual properties (parsed by `hepcep::parse_parameters`). Under MPI the model is
distributed across ranks (`mpirun -n N ...`); `main.cpp` sets up `RepastProcess` and
the schedule runner.

## Tests
Build the `tests` target and run the resulting test binary. Tests use GoogleTest and
read from `test_data/`.

## Architecture
Entry: `main.cpp` → `run.cpp` (`runModel`) → `HCModel` (`HCModel.h/.cpp`), which
extends `chi_sim::AbstractModel<HCPerson, HCPlace, HCPersonData>`.

- `HCModel` owns the schedule, the PWID network, zones, and per-tick behavior
  (`step()`): person steps, `zoneCensus()`/`performLinking()` network rewiring,
  `daa_treatment()`, `opioid_treatment()`, and `generateArrivingPersons()`.
- `HCPerson` (`HCPerson.h/.cpp`) is the agent: demographics, injection intensity,
  syringe source, drug in/out degree, and an owned `Immunology`.
- `Immunology` (`Immunology.h`) is an abstract interface with two implementations:
  - `APK_Immunology` — the original compartmental infection model.
  - `VK_Immunology` + `ViralKinetics` + `VKProfile` — viral-kinetics infection model
    driven by the `viral.kinetics.*` CSV inputs. `immunology.type` selects which;
    `config/model.props` defaults to `VK`.
- Network: `Network.h`, `network_utils.*`, `NetworkBuilder`, and the GML parser.
- Data loading: `PersonCreator` + `PersonDataLoader` read the CNEP+ catalog or ERGM
  catalog/edge-list depending on `pwid.data.input.type`.
- Output: `Statistics` (aggregate + per-tick stats), `AggregateStats`, `LogType`,
  `FileOut`/`FileSink`, and `serialize.*` for saving/resuming network state.

## Configuration
`config/model.props` drives a run. Notable switches:
- `immunology.type` — `APK` or `VK`.
- `pwid.data.input.type` — `CNEP+` or `ERGM` (ERGM also needs
  `pwid.network.edges.input.file`).
- `treatment_enrollment_strategy` — `INFECTED_ONLY` or `ALL_PWID`; `ALL_PWID` uses
  periodic HCV screening via `hcv_screening_interval`.
- DAA enrollment split across `treatment_enrollment_probability_{unbiased,HRP,
  fullnetwork,inpartner,outpartner}`, plus `prob_daa_harm_reduction` /
  `daa_harm_reduction_duration`.
- MOUD via `opioid_treatment_*` (methadone, naltrexone, buprenorphine) and
  `opioid_treatment_access_scenario` (`REAL`, `SCENARIO_1`, `SCENARIO_2`).
- `resume.from.saved.net` / `resume.from.saved.net.file` for checkpoint resume.

## Conventions and gotchas
- `src/module.mk` lists compiled sources; add new `.cpp` files there.
- `hepcep_model_wrapper.cpp` and `*.bu` are legacy/backup and not in `module.mk`.
- Generated parser files (`gml.tab.c`, `lex.yy.c`) are checked in; regenerate from
  `grammar/` only if the grammar changes.
- `Release/` and `Debug/` are gitignored build artifacts.

## Relationship to the Python port
`../hepcep4py` reimplements this model on Repast4Py. It adds an HCV **vaccine trial**
module that does not exist here, but it currently lacks several features present in
this C++ version, including HCV screening / the `ALL_PWID` treatment strategy, ERGM
network input, dynamic harm-reduction enrollment, DAA enrollment methods beyond
`unbiased`, network checkpoint/resume, and multi-rank distribution. The C++ model
remains the reference implementation. See `../hepcep4py/CLAUDE.md` for the full
comparison.
