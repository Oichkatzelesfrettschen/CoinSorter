# CoinSorter / Superforce

Comprehensive C99 toolkit for:

* Coin change optimization (minimal coin count plus alternative objective minimization: total mass, cumulative diameter, planar area).
* Currency metadata (mass, diameter, composition) with canonicality audit.
* Physics sandbox (phi^4 beta coefficients, anomalous dimension approximation, Casimir force with thermal & modulation toggles across multiple planetary environments).
* Procedural simulation (fractal Brownian motion terrain via diamond–square, Poisson (Jacobi) solver, deflection vector field derivation, simple PPM dumps).
* Lightweight MLP (single hidden layer) for demonstration & live loss viewing in ncurses UI.
* Multi-interface: CLI tools, minimal interactive text UI, optional ncurses advanced UI (mouse + color + live progress + JSON export, physics overlay, value noise, area objective, vector arrows, residual heatmap).

All implemented in portable C99 with zero external deps unless `BUILD_NCURSES_UI=ON` (ncurses).

Supported currencies: USD, EUR, CAD, AUD, NZD, CNY (extendable via `coin_systems.c`).

---

## Contents

1. Quick Start
2. Build Matrix & Options
3. Executables Overview
4. Coin Algorithms & Objectives
5. JSON Output Schema
6. Physics & Environments
7. Simulation Components
8. MLP Demo
9. Interactive UIs (text + ncurses)
10. Color Handling
11. Library API Surface
12. Testing
13. Extending the System
14. Roadmap / Ideas
15. License

---

## 1. Quick Start

\n```sh
cmake -S . -B build -DBUILD_TESTS=ON -DBUILD_SUPERFORCE=ON -DBUILD_UI=ON -DBUILD_NCURSES_UI=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Run a sample:
\n```sh
./build/bin/coinsorter amt=137 sys=usd --json opt=mass
./build/bin/superforce --sim --fbm-ppm --poisson --vectors
./build/bin/superforce_ui       # lightweight interactive
./build/bin/superforce_ncui     # ncurses UI (if built)
```

Install (optional):
\n```sh
cmake --install build --prefix /usr/local
```

---

## 2. Build Matrix & Options

| CMake Option | Default | Purpose |
|--------------|---------|---------|
| `BUILD_SUPERFORCE` | ON | Build unified physics+simulation demo binary `superforce`. |
| `BUILD_UI` | ON | Build dependency-free interactive text UI `superforce_ui`. |
| `BUILD_NCURSES_UI` | OFF | Build advanced ncurses UI `superforce_ncui` (mouse, color panes, progress, JSON export). |
| `BUILD_TESTS` | ON | Build test executables & enable `ctest`. |
| `ENABLE_DOXYGEN` | OFF | Generate API docs (requires Doxygen). |

### Documentation

When `-DENABLE_DOXYGEN=ON` is provided to CMake, Doxygen will parse all headers and sources (annotated with `\file`, `\brief`, and per-symbol tags) and emit HTML documentation under `build/docs/html/index.html`.

Minimal example:

```bash
cmake -S . -B build -DENABLE_DOXYGEN=ON
cmake --build build --target docs
open build/docs/html/index.html
```

All public APIs are now annotated; internal static helpers include brief summaries for clarity.

Artifacts land in `build/bin` and `build/lib`.

#### Fast Build Script

An optional helper is provided at `scripts/build.sh` which:

* Uses Ninja + ccache when available.
* Auto-selects 75% of logical cores.
* Applies `-O3 -flto -march=native` (native tunable via `NATIVE=0`).

Usage:

```bash
chmod +x scripts/build.sh
scripts/build.sh -DBUILD_NCURSES_UI=ON -DBUILD_TESTS=ON
ctest --test-dir build --output-on-failure
```

---
 
## 3. Executables Overview

* `coinsorter` – Coin change solver & benchmarking (greedy vs DP, objective switching, JSON serialization).
* `superforce` – All-in-one demo: physics numbers, environment parameters, simulation generation (fBm, Poisson, vectors), coin optimization.
* `superforce_ui` – Keyboard-driven minimal interface (ANSI color optional).
* `superforce_ncui` – Rich ncurses interface (mouse clicks, progress bar style iterative redraw for Poisson, live MLP epochs, JSON export with one key).
* Tests: `test_basic`, `test_sim`, `test_coin_adv` (see section 12).

---
 
## 4. Coin Algorithms & Objectives

Algorithms:
 
* Greedy (fast; optimal only for canonical systems).
* DP minimal coin count (`dp_make_change`).
* DP with alternate objective weighting: `dp_make_change_opt(mode=OPT_MASS|OPT_DIAMETER|OPT_AREA)` minimizing sum of masses, diameters, or planar area with coin-count tiebreak.
* Canonicality audit: `audit_canonical` brute-forces search bound (default product of top two denominations) verifying greedy optimality or reporting a counterexample.

Objectives / Strategy Labels:
 
* `greedy` – Greedy algorithm counts.
* `dp` – Minimal coin count.
* `dp-mass` – Mass-minimizing solution (with fallback weighting if missing metadata).
* `dp-diam` – Diameter-minimizing solution.
* `dp-area` – Planar area minimizing (π(d/2)^2 aggregate) with fallback.
Example (area) JSON:

```json
{
{
	"system": "usd",
	"amount": 137,
	"strategy": "dp-area",
	"objective": "area",
	"objective_value": 1234.56,
	"coins": [
		{"code": "25c", "value": 25, "count": 5},
		{"code": "10c", "value": 10, "count": 1},
		{"code": "5c", "value": 5, "count": 0},
		{"code": "1c", "value": 1, "count": 2}
	]
}
```

---
 
## 5. JSON Output Schema

Produced by `format_change_json` and available in CLI & ncurses export (`ncui_change.json`). Example fields:

```json
{
{
	"system": "usd",
	"amount": 137,
	"strategy": "dp-mass",
	"version": "<semver>",
	"mass_g": 15.4060,
	"diameter_mm": 123.45,
	"total_coins": 7,
	"objective": "mass",
	"objective_value": 15.4060,
	"coins": [
		{"code": "25c", "value": 25, "count": 5},
		{"code": "10c", "value": 10, "count": 1},
		{"code": "5c", "value": 5, "count": 0},
		{"code": "1c", "value": 1, "count": 2}
	]
}
```

Notes:

* `mass_g` / `diameter_mm` are -1 internally if unknown; serialized as 0.0 when absent.
* `objective` disambiguates which optimization target produced `objective_value`.

---
 
## 6. Physics & Environments

Environments: `earth`, `moon`, `mars`, `orbit` – each with gravity & placeholder thermodynamic parameters used by Casimir calculations & displays.

Physics functions (simplified research constants):
 
* `beta1`, `beta2` – phi^4 beta expansion coefficients.
* Casimir force base, thermal, and modulated contributions (toggled in unified binary via flags).

---
 
## 7. Simulation Components

* fBm heightfield via diamond–square (`fbm_diamond_square`) with fallback noise generator.
* Alternative value noise (`generate_value_noise`) multi-octave tileable field.
* Poisson solver: Jacobi iterations over generated field (residual tracked and displayed in ncurses status & simulation pane).
* Deflection vector field: gradient-based derivation (`compute_deflection`).
* Optional PPM output for visual inspection (CLI flag in `superforce`).

---
 
## 8. MLP Demo

Single hidden-layer network (ReLU) with:
 
* `mlp_init`, `mlp_forward`, `mlp_train_epoch`, `mlp_free`.
* Ncurses UI key `m` runs a brief training loop printing epoch & loss into the change pane footer.

---
 
## 9. Interactive UIs

 
### Text UI (`superforce_ui`)

Keyboard shortcuts (subset):

* `h` help, `o` cycle optimization, `c` cycle currency, `e` cycle environment, `f` generate fBm, `p` Poisson solve, `v` vector field, `m` MLP demo, `q` quit.

### Ncurses UI (`superforce_ncui`)

Features:

* Separate panes: status, change solution, simulation preview ASCII, help.
* Mouse: click bracketed buttons (`[+]`, `[-]`, `[opt]`, `[sys]`, `[env]`, `[fbm]`, `[val]`, `[solve]`, `[json]`, `[pois]`, `[vec]`, `[mlp]`, `[phys]`).
* Keys mirror mouse: `+ - o y e f n c j p v m g` etc.
* `n` or `[val]` generates value noise in place of fBm; `f` or `[fbm]` regenerates diamond–square fBm.
* Optimization cycle includes area objective.
* Poisson: iterative batches show progress (residual at bottom of sim pane & status line).
* JSON export: key `j` or `[json]` writes `ncui_change.json` in current directory.
* Live MLP: key `m` prints epoch/loss updates.
* Physics overlay toggle & area objective cycling.
* Value noise generation (`[val]` button or `n` key) alongside fBm.
* Vector arrow overlay toggle (`v` key / `[vec]` button) renders 8‑direction arrows colored by magnitude.
* Residual heatmap toggle (`r` key) pseudo‑colors Poisson residual magnitude atop the field (legend implicit; hotter colors = higher residual).
* Status line flags: `v` indicates arrows active, `r` indicates residual heatmap active.
* Color pairs differentiate panes & overlays (requires terminal color support).

---
 
## 10. Color Handling

ANSI color auto-detected; disabled if:

* `--no-color` CLI flag passed (where supported), or
* `NO_COLOR` environment variable set.

Global toggle logic resides in `color.c` with `color_enabled` check used by higher-level output.

---
 
## 11. Library API Surface

Headers under `include/` expose:

* `coins.h` (systems, algorithms, JSON, optimization modes, audit)
* `env.h` (environment descriptors)
* `beta.h`, `casimir.h` (physics) – if present
* `simulation.h` (fBm, Poisson, vectors, MLP)
* `color.h` (ANSI toggling – internal friendly)

Link against `coins_core` for all functionality.

---
 
## 12. Testing

Tests (when `BUILD_TESTS=ON`):

* `test_basic` – beta coefficients & greedy correctness.
* `test_sim` – fBm + Poisson residual reduction, MLP convergence, NO_COLOR enforcement.
* `test_coin_adv` – advanced coin objective consistency (count/mass/diameter/area), JSON schema fields (objective, total_coins), tiny buffer failure path, canonicality audit.

Run all:

```bash
ctest --test-dir build --output-on-failure
```

* Add new currency: append `CoinSpec` array & entry in `coin_systems.c`.
* New objective: augment `OptimizeMode` enum, extend DP weighting logic & strategy label mapping.
* Alternative terrain / noise: implement function and integrate into UI generation path.
* Additional physics observables: create new source file & add to `coins_core` library list in `CMakeLists.txt`.
* Enhanced ncurses overlays (vector arrows, colored residual heatmap) – future.

Contribution guidelines (lightweight):

1. Keep to C99, portable & warning clean (`-Wall -Wextra -Werror`).
2. Add/adjust tests for user-visible behavior changes.
3. Document new CLI flags & JSON fields here.

---
 
## 14. Roadmap / Ideas

* Configurable Poisson iteration count & dynamic progress bar.
* Multi-threaded (OpenMP) optional build for large DP or solver sizes.
* Additional optimization objectives (coin surface area, cost, rarity weighting).
* Live graph of residual/epoch loss (sparkline) in ncurses.
* Doxygen + Sphinx narrative docs (ENABLE_DOXYGEN).
* WASM port of core algorithms for web demo.
* Extended observables (e.g., vorticity, divergence, curvature) with selectable overlays.

---
 
## 15. License

MIT (provide LICENSE file – currently assume MIT usage).

---
 
## FAQ (Selected)

**Q:** Why minimize mass / diameter?  
**A:** Illustrates multi-objective DP layering and shows how metadata enriched systems enable varied optimization criteria (e.g., minimizing carried weight).

**Q:** Is JSON stable?  
**A:** Core fields (`system`, `amount`, `strategy`, `coins`) stable; added `objective`/`objective_value` for clarity. New fields will append—avoid strict schema position assumptions.

**Q:** Large amounts performance?  
**A:** DP is O(N * amount). For very large amounts consider a bounded search or hybrid heuristics; future roadmap includes performance tuning.

**Q:** Missing mass/diameter values?  
**A:** They default to weight 1.0 internally to preserve reachability; JSON exports 0.0 so clients can detect absence.

---
Happy sorting & simulating!
