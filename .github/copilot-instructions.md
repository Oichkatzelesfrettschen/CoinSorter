# CoinSorter / Superforce

CoinSorter is a comprehensive C99 toolkit for coin change optimization, physics simulation, and interactive UI demonstration. It provides multiple algorithms for coin change problems with different optimization objectives, plus physics simulations and interactive interfaces.

**ALWAYS follow these instructions first and fallback to search or bash commands only when you encounter unexpected information that does not match the info here.**

## Working Effectively

### Bootstrap, Build, and Test the Repository:

**NEVER CANCEL builds or tests** - while fast, allow them to complete for reliable results.

```bash
# Standard build (recommended for development):
cmake -S . -B build -DBUILD_TESTS=ON -DBUILD_SUPERFORCE=ON -DBUILD_UI=ON -DBUILD_NCURSES_UI=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

**Timing expectations:**
- Configure: ~0.2-1.1 seconds (cold start slower). Set timeout to 30+ seconds.
- Build: ~0.5-3 seconds (parallel compilation). Set timeout to 60+ seconds.
- Tests: ~1.5-6 seconds (coin_adv test takes ~6s for canonicality checking). Set timeout to 120+ seconds.

**Alternative fast build script (optimized with Ninja + ccache):**
```bash
chmod +x scripts/build.sh
scripts/build.sh -DBUILD_NCURSES_UI=ON -DBUILD_TESTS=ON
# Fast build takes ~2.7 seconds total (much faster than standard build)
# Test time: ~1.6 seconds (faster due to optimization)
```

**Build with documentation:**
```bash
# Requires: sudo apt-get install -y doxygen graphviz
cmake -S . -B build -DENABLE_DOXYGEN=ON
cmake --build build --target docs
# Doc generation takes ~0.4-1 second
# View: build/docs/html/index.html
```

### Dependencies and Installation:

**Required dependencies:**
```bash
# C compiler and build tools (usually pre-installed):
sudo apt-get install -y build-essential cmake
# Minimum versions: cmake ≥3.16, GCC or any C99 compiler
```

**Optional dependencies:**
```bash
# For ncurses UI (BUILD_NCURSES_UI=ON):
sudo apt-get install -y libncurses5-dev

# For documentation (ENABLE_DOXYGEN=ON):
sudo apt-get install -y doxygen graphviz
```

**Available build tools (automatic detection):**
- `ninja` (preferred, faster builds - auto-detected by fast build script)
- `make` (fallback)
- `ccache` (automatic if available, significantly speeds up rebuilds)

## Validation

**ALWAYS manually validate changes with these scenarios:**

```bash
# 1. Basic coin change solving:
./build/bin/coinsorter 137 usd

# 2. JSON output validation (should be valid JSON):
./build/bin/coinsorter 137 usd --json | python3 -m json.tool

# 3. Different optimization modes:
./build/bin/coinsorter 137 usd --opt=mass
./build/bin/coinsorter 137 usd --opt=diam
./build/bin/coinsorter 137 usd --opt=area

# 4. Multiple currency systems:
./build/bin/coinsorter 67 eur
./build/bin/coinsorter 67 cad

# 4a. Additional currency systems:
./build/bin/coinsorter 100 aud
./build/bin/coinsorter 100 nzd  
./build/bin/coinsorter 100 cny

# 5. Simulation with file output:
./build/bin/superforce --sim --fbm-ppm --poisson --vectors
# Verify *.ppm files are generated

# 6. Interactive UI (should start without crashing):
echo "q" | ./build/bin/superforce_ui
# For ncurses UI (if built):
timeout 3 ./build/bin/superforce_ncui || echo "OK"

# 8. Test additional features:
./build/bin/coinsorter 137 usd --audit  # Test canonicality audit
./build/bin/coinsorter --selftest       # Run internal validation tests
```

**ALWAYS run these commands before finishing any change:**
```bash
# Clean build and test:
rm -rf build
cmake -S . -B build -DBUILD_TESTS=ON -DBUILD_SUPERFORCE=ON -DBUILD_UI=ON -DBUILD_NCURSES_UI=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure

# Validation scenarios (above)
```

## Build Matrix & Options

| CMake Option | Default | Purpose |
|--------------|---------|---------|
| `BUILD_SUPERFORCE` | ON | Build unified physics+simulation demo binary `superforce` |
| `BUILD_UI` | ON | Build dependency-free interactive text UI `superforce_ui` |
| `BUILD_NCURSES_UI` | OFF | Build advanced ncurses UI `superforce_ncui` (requires ncurses) |
| `BUILD_TESTS` | ON | Build test executables & enable `ctest` |
| `ENABLE_DOXYGEN` | OFF | Generate API docs (requires doxygen+graphviz) |

## Executables Overview

After building, artifacts land in `build/bin/` and `build/lib/`:

### Core Applications:
- **`coinsorter`** - Main CLI tool for coin change optimization
  ```bash
  ./build/bin/coinsorter [amount] [system] [--json] [--audit] [--opt=count|mass|diam|area]
  # Example: ./build/bin/coinsorter 137 usd --json
  # Note: --help shows usage but exits with code 1
  # Note: --version prompts for input instead of showing version
  ```

- **`superforce`** - Unified demo with physics, simulation, and coin solving
  ```bash
  ./build/bin/superforce [--sim] [--fbm-ppm] [--poisson] [--vectors]
  ```

### Interactive UIs:
- **`superforce_ui`** - Lightweight text-based interactive interface
- **`superforce_ncui`** - Advanced ncurses UI with mouse support (if built)

### Test Executables:
- `test_basic` - Beta coefficients & greedy correctness
- `test_sim` - fBm + Poisson residual reduction, MLP convergence
- `test_coin_adv` - Advanced coin objective consistency, JSON schema validation
- `test_area` - Area optimization testing
- `test_observables` - Physics observables validation

## Supported Features

### Currency Systems:
`usd`, `eur`, `cad`, `aud`, `nzd`, `cny` (extensible via `src/coin_systems.c`)

### Optimization Objectives:
- `count` (default) - Minimize total number of coins
- `mass` - Minimize total mass
- `diam` - Minimize cumulative diameter
- `area` - Minimize total planar area

### File Outputs:
- JSON coin change solutions
- PPM image files from simulations (`fbm.ppm`, `poisson_phi.ppm`, `fbm_vectors.ppm`)
- Doxygen HTML documentation (`build/docs/html/`)

## Installation (Optional)

```bash
sudo cmake --install build --prefix /usr/local
# Installs to: /usr/local/bin/, /usr/local/lib/, /usr/local/include/
```

## Common Issues and Solutions

### Build Failures:
```bash
# Missing ncurses (for BUILD_NCURSES_UI=ON):
sudo apt-get install -y libncurses5-dev

# Missing doxygen dependencies:
sudo apt-get install -y doxygen graphviz

# Clean and rebuild:
rm -rf build && cmake -S . -B build [options] && cmake --build build --parallel
```

### Test Failures:
- `test_coin_adv` may take 5-6 seconds (canonicality checking) - this is normal
- Tests may fail if core library wasn't built correctly - rebuild from scratch
- Missing test executables usually indicate CMake cache issues - clean build

### Runtime Issues:
```bash
# Invalid currency/amount combinations:
# Some currencies can't make change for certain amounts (by design)
# Invalid inputs show usage and exit with code 1

# Color output issues:
export NO_COLOR=1  # Disable ANSI colors

# Command behavior notes:
# --help shows usage but exits with status 1 (not 0)
# --version prompts for input instead of displaying version
# Invalid arguments display usage with available systems list
```

## Key Project Structure

```
├── src/                    # Source files
│   ├── coinsorter.c       # Main CLI application  
│   ├── coin_algorithms.c  # Core optimization algorithms
│   ├── coin_systems.c     # Currency definitions
│   ├── superforce_*.c     # Interactive UIs
│   └── *.c               # Physics & simulation modules
├── include/               # Public headers
├── tests/                 # Test suite
├── scripts/build.sh       # Fast build script
└── CMakeLists.txt        # Build configuration
```

## Development Guidelines

- Maintain C99 compatibility
- Code compiles with `-Wall -Wextra -Werror`
- Add tests for new functionality
- Update this file when adding new features or changing build requirements
- All executables should handle invalid input gracefully
- JSON output must remain valid and schema-compatible

## Performance Notes

- Configure times: ~0.2-1.1 seconds (first run slower)
- Build times: ~0.5-3 seconds (parallel compilation)  
- Test suite completes in ~1.5-6 seconds (coin_adv test takes ~6s)
- Fast build script: ~2.7 seconds total (configure + build + test ~4.3s total)
- Documentation generation: ~0.4-1 second
- Simulation file generation is near-instantaneous
- Most operations are suitable for interactive use