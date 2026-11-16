# EtaDecayer

EtaDecayer is an exclusive η/η′ decayer for [Pythia 8](https://pythia.org). Every η (PDG id 221) or η′ (331) in a Pythia event decays into a single chosen final state, with a matrix element that includes the measured or calculated transition form factors. Examples are η → μ⁺μ⁻e⁺e⁻, 4μ and 4e, which use the full direct–exchange interference, and η′ → ηπ⁺π⁻, which uses the BESIII Dalitz-plot density.

EtaDecayer is a `Pythia8::DecayHandler`, registered with `Pythia::setDecayPtr()`. It needs only Pythia 8 and ROOT.

## Decay modes

Set the mode with `EtaDecayer:mode`. "P" means the η or the η′, selected with `EtaDecayer:parent = 221 | 331`.

| mode | decay | model |
|---|---|---|
| `2mu` | P → μ⁺μ⁻ | isotropic |
| `2egamma`, `2mugamma` | P → ℓ⁺ℓ⁻γ | Kroll–Wada with the transition form factor (TFF) |
| `2mu2e` | P → μ⁺μ⁻e⁺e⁻ | factorized double-virtual TFF |
| `4e`, `4mu` | P → ℓ⁺ℓ⁻ℓ⁺ℓ⁻ | direct − exchange interference, TFF applied to each diagram |
| `2e2pi`, `2mu2pi` | P → π⁺π⁻ℓ⁺ℓ⁻ | magnetic angular term, π⁺π⁻γ* amplitude of arXiv:2210.14925 |
| `eta2pi`, `eta2pi0` | η′ → ηπ⁺π⁻, ηπ⁰π⁰ | Dalitz plot 1 + aY + bY² + dX², BESIII arXiv:1709.04627 (η′ only) |
| `2muA` | P → μ⁺μ⁻A | A = dark photon or axion-like particle (ALP); `EtaDecayer:A_id`, `EtaDecayer:A_type` |
| `2piA` | P → π⁺π⁻A | the same |

The models and their sources:

| vertex | model | reference |
|---|---|---|
| η TFF | Padé approximant P⁷₁ fitted to data | Escribano, Masjuan, Sanchez-Puertas, arXiv:1504.07742 |
| η′ TFF | Padé approximant P⁶₁ below 0.70 GeV, ρ+ω+φ vector-meson dominance above | Escribano, Gonzalez-Solis, arXiv:1511.04916 (coefficients from arXiv:1307.2061) |
| P → π⁺π⁻γ* | amplitude with a ρ Breit–Wigner | Zillinger, Kubis, Sanchez-Puertas, arXiv:2210.14925 |
| angular structure | — | Petri, arXiv:1010.2378 |
| η′ → ηππ Dalitz parameters | BESIII general representation | arXiv:1709.04627, Table II |

[`EtaDecayer.h`](include/EtaDecayer/EtaDecayer.h) documents every setting, the order of the daughters in the event record, the mass reach of the A modes and the treatment of the η′ mass window. [`EtaDecayerModels.h`](include/EtaDecayer/EtaDecayerModels.h) describes the models and [`EtaDecayerSamplers.h`](include/EtaDecayer/EtaDecayerSamplers.h) the samplers.

**What the samples give.** The samples are *forced-exclusive*: they give the shape of the chosen decay, and the branching fraction has to be applied separately. Radiative (QED) corrections are not included. For ηπ⁰π⁰, the cusp at the π⁺π⁻ threshold is not modelled; it is a few-percent effect in the first m(ππ) bins.

**One parent per event.** By default every parent in the event is forced. With `EtaDecayer:onePerEvent = on`, exactly one parent per event is forced, chosen at random, and the others decay with Pythia's decay table. In that case, call `decayOnePerEvent()` after every `pythia.next()`, and weight each event by its number of parents (see `EtaDecayer.h`).

## Contents

```
include/EtaDecayer/
  EtaDecayer.h            the DecayHandler, its settings and assumptions
  EtaDecayerModels.h      form factors, amplitudes, accept-reject bounds
  EtaDecayerSamplers.h    unweighted samplers of each decay
  EtaDecayerException.h   exception thrown on invalid settings or failed generation
src/EtaDecayer.cc
test/testEtaDecayer.cc    Catch2 v3 unit tests
examples/etatest.cc       example program and check of every mode
build.sh                  builds etatest and the unit tests in build/
run_checks.sh             unit tests + etatest for all modes
```

## Requirements

- A C++17 compiler.
- Pythia 8.3. EtaDecayer has been used with Pythia 8.306, 8.309 and 8.317.
- ROOT 6, which provides `TLorentzVector`.
- Catch2 v3, needed only for the unit tests.

On a machine with CVMFS, one LCG view provides all of these, for example:

```bash
source /cvmfs/sft.cern.ch/lcg/views/LCG_109/x86_64-el9-gcc14-opt/setup.sh
```

## Build and check

```bash
./build.sh          # needs pythia8-config and root-config in PATH
./run_checks.sh     # unit tests + every mode, about 1 minute
```

`build.sh` reads two environment variables:

- `CXX`: the compiler (default `g++`).
- `CATCH2_DIR`: the Catch2 v3 installation prefix. The default is the prefix of `pythia8-config`, which is correct for an LCG view.

If Catch2 is not found, only `etatest` is built.

At run time Pythia reads its data files from the directory in `PYTHIA8DATA`. LCG views set this variable. Otherwise, set it with `export PYTHIA8DATA=$(pythia8-config --xmldoc)`; `run_checks.sh` does this itself when the variable is unset.

`run_checks.sh` runs the unit tests (197 457 assertions). It then runs `etatest` for each of the 22 mode and parent combinations, once with every parent forced and once with one per event. Each run checks the final state of every forced decay and momentum conservation. The script prints `PASS` or `FAIL` for each run and exits with a non-zero status if any run fails.

To run a single mode:

```
build/etatest <mode>[:221|:331] [nEvents=1000] [seed=1] [onePerEvent=0]
```

For example:

```bash
build/etatest 2mu2e 1000             # eta -> mu+ mu- e+ e-
build/etatest 4mu:331 1000           # eta' -> 4 mu
build/etatest eta2pi0 1000 7 1       # eta' -> eta pi0 pi0, one eta' per event, seed 7
```

`etatest` generates minimum-bias pp collisions at 13.6 TeV. For `2mu2e` it also prints ⟨cos 2Φ⟩, the modulation of the angle between the μ⁺μ⁻ and e⁺e⁻ decay planes. With 200 000 events (1.2 × 10⁶ η decays) it gives −0.0407 ± 0.0006 (seed 1, about 9 minutes); the analytic value for the full μ⁺μ⁻e⁺e⁻ phase space is −0.0403. For `2muA` and `2piA` it defines a 0.25 GeV dark photon (PDG id 9000001) that decays to e⁺e⁻.

## Using EtaDecayer in your own Pythia program

Compile `src/EtaDecayer.cc` with `-Iinclude`, as `build.sh` does. Then:

```cpp
#include "Pythia8/Pythia.h"
#include "EtaDecayer/EtaDecayer.h"

Pythia8::Pythia pythia;
EtaDecayer::addSettings(pythia.settings);   // registers the EtaDecayer:* settings

pythia.readString("EtaDecayer:filter = on");
pythia.readString("EtaDecayer:parent = 221");
pythia.readString("EtaDecayer:mode = 2mu2e");
auto decayer = std::make_shared<EtaDecayer>(&pythia, &pythia.settings);
pythia.setDecayPtr(decayer, {221});   // the same PDG id as EtaDecayer:parent
pythia.init();

while (...) {
  if (!pythia.next()) continue;
  if (!decayer->decayOnePerEvent()) continue;   // does nothing unless EtaDecayer:onePerEvent = on
  ...
}
```

Invalid settings, such as an unknown mode or a mode that is kinematically closed, throw `etadecayer::Exception` (a `std::exception`) from the constructor.

## Validation

- The unit tests check the form factors, the kinematics of every mode, the η′ → ηππ Dalitz-plot moments against the BESIII density, the one-parent-per-event option and the rejection of invalid settings.
- Fits to 10⁶ decays per mode recover the input form factors and Dalitz parameters.
- The decay distributions were compared with those of Pluto, EvtGen and Pythia's own decay tables.

## License and citation

EtaDecayer is distributed under the Apache License 2.0 (see [`LICENSE`](LICENSE)). If you use it, please cite this package; [`CITATION.cff`](CITATION.cff) has the metadata. Please also cite the physics inputs listed above.
