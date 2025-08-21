/**
 * \file coins.h
 * \brief Core currency / coin change API: systems, algorithms, optimization
 * modes, utilities.
 *
 * The API supports:
 *  - Multiple predefined coin systems (USD, EUR) with physical metadata.
 *  - Greedy and dynamic programming change algorithms.
 *  - Multi-objective optimization (minimize coin count, total mass, or total
 * diameter).
 *  - Canonical system audit to check greedy optimality up to a search bound.
 *  - Serialization of results to a compact JSON representation for downstream
 * tooling.
 */
#ifndef COINS_H
#define COINS_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** \brief Single coin denomination specification. */
typedef struct {
  int value;               /**< Integer value in smallest unit (e.g., cents). */
  const char *code;        /**< Short code (e.g., "25c"). */
  const char *name;        /**< Human‑readable name. */
  double mass_g;           /**< Mass in grams (0 if unknown). */
  double diameter_mm;      /**< Diameter in mm (0 if unknown). */
  const char *composition; /**< Optional alloy/composition description. */
} CoinSpec;

/** \brief Group of coin specs forming a currency system. */
typedef struct {
  const char *system_name; /**< Identifier e.g. "usd". */
  const CoinSpec *coins;   /**< Array sorted in descending value order. */
  size_t ncoins;           /**< Number of denominations. */
  int smallest_unit;       /**< Scaling factor (1 for cents). */
  int canonical_hint; /**< 1 if known canonical; 0 if unknown/unverified. */
} CoinSystem;

/* Predefined systems */
/** \brief Retrieve a predefined system by name ("usd", "eur"). */
const CoinSystem *get_coin_system(const char *name);
/** \brief Print available systems to stdout. */
void list_systems(void);

/* Core algorithms */
/** \brief Greedy algorithm (fast; optimal only if system canonical). */
int greedy_make_change(const CoinSystem *sys, int amount, int *counts);
/** \brief Dynamic programming minimal coin count solution. */
int dp_make_change(const CoinSystem *sys, int amount, int *counts);

/* Audit canonical optimality; returns 1 if greedy optimal up to bound, else 0.
   If a counterexample is found and ex_amount!=NULL it is stored there. */
/** \brief Audit greedy optimality up to a bound (0 => heuristic product of top
 * two denominations). */
int audit_canonical(const CoinSystem *sys, int search_limit, int *ex_amount);

/* Utility: serialize result to JSON (buffer must be provided). */
/** \brief Serialize change result (counts aligned to system order) into JSON.
 */
int format_change_json(const CoinSystem *sys, int amount, const int *counts,
                       const char *strategy, const char *version, char *buf,
                       size_t buflen);

/* Compute total mass (grams) for counts; returns negative if mass data missing
 */
/** \brief Compute total mass (grams); returns negative if insufficient
 * metadata. */
double total_mass(const CoinSystem *sys, const int *counts);
/** \brief Compute cumulative diameter sum (mm); returns negative if
 * insufficient metadata. */
double total_diameter(const CoinSystem *sys, const int *counts);
/** \brief Compute total planar area (mm^2) approximating coins as circles;
 * returns negative if insufficient metadata. */
double total_area(const CoinSystem *sys, const int *counts);

/* Optimization mode */
/** \brief Optimization objective (keep ordering stable for UI cycling). */
typedef enum {
  OPT_COUNT = 0,    /**< Minimize number of coins. */
  OPT_MASS = 1,     /**< Minimize total mass (g). */
  OPT_DIAMETER = 2, /**< Minimize cumulative diameter (mm). */
  OPT_AREA = 3,     /**< Minimize planar area (pi*(d/2)^2). */
  OPT_MODE_COUNT    /**< Sentinel (number of modes). */
} OptimizeMode;

/* DP with optimization mode (count, mass, diameter). Returns 0 success. */
/** \brief DP minimizing selected objective; ties resolved by fewer coins. */
int dp_make_change_opt(const CoinSystem *sys, int amount, int *counts,
                       OptimizeMode mode);

#ifdef __cplusplus
}
#endif

#endif /* COINS_H */
