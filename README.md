# CoinSorter / Superforce

Modular C99 toolkit combining coin change optimization (count, mass, diameter), physics (phi^4 beta / anomalous dimension, Casimir force with thermal toggle), procedural fractal terrain (fBm), Poisson (Jacobi) solver, deflection vector field rendering, and a small MLP scaffold.

## Build

```sh
cmake -S . -B build -DBUILD_TESTS=ON -DBUILD_SUPERFORCE=ON -DBUILD_UI=ON
cmake --build build --parallel
```

## Run

List of common invocations:

- `coinsorter amt=123 sys=usd --json opt=mass` optimize for mass.
- `superforce --physics env=moon --no-thermal` physics sample under lunar gravity.
- `superforce --sim fbmSize=257 fbmH=0.7 --fbm-ppm --poisson --vectors` generate fBm, solve Poisson, and vector overlay.
- `superforce_ui` interactive text UI (type `h` for help).

## Install

```sh
cmake --install build --prefix /usr/local
```

Installs library (`libcoins_core.a`), headers, and executables.

## CI

GitHub Actions workflow in `.github/workflows/ci.yml` builds, tests, and installs on pushes / PRs.

## Features Snapshot

- Greedy + DP coin change; multi-objective DP (count/mass/diameter).
- Canonical system audit heuristic.
- USD & EUR systems with mass, diameter, composition metadata.
- Physics: phi^4 beta coefficients, anomalous dimension; Casimir with modulation + thermal toggle.
- Simulation: diamond-square fBm, fallback noise, Poisson Jacobi solver, deflection vectors, PPM outputs.
- MLP: init/forward/train (demo scaffold only).
- Environments: earth, moon, mars, orbit (gravity/temperature/pressure).
- Deterministic xorshift RNG.

## Roadmap Ideas

- Colorized terminal output (ANSI with fallback).
- More tests (Poisson residual reduction, mass/diameter objective regression cases).
- Additional currencies & physical data.
- Expose MLP example in UI.
- Documentation via Doxygen + Sphinx (ENABLE_DOXYGEN option).

## License

MIT (add LICENSE file if missing).
