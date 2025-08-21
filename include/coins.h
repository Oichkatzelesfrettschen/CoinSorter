#ifndef COINS_H
#define COINS_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Coin denomination specification. */
typedef struct {
  int value;               /**< integer value in smallest unit (e.g., cents) */
  const char *code;        /**< short code e.g. "25c" */
  const char *name;        /**< human-readable name */
  double mass_g;           /**< mass in grams (0 if unknown) */
  double diameter_mm;      /**< diameter in mm (0 if unknown) */
  const char *composition; /**< optional alloy/composition description */
} CoinSpec;

/** Group of coin specs forming a currency system. */
typedef struct {
  const char *system_name; /**< e.g., "usd" */
  const CoinSpec *coins;   /**< array descending by value */
  size_t ncoins;           /**< number of coins */
  int smallest_unit;       /**< scaling factor (1 for cents) */
  int canonical_hint;      /**< 1 if known canonical; 0 unknown */
} CoinSystem;

/* Predefined systems */
const CoinSystem *get_coin_system(const char *name);
void list_systems(void);

/* Core algorithms */
int greedy_make_change(const CoinSystem *sys, int amount, int *counts);
int dp_make_change(const CoinSystem *sys, int amount, int *counts);

/* Audit canonical optimality; returns 1 if greedy optimal up to bound, else 0.
   If a counterexample is found and ex_amount!=NULL it is stored there. */
int audit_canonical(const CoinSystem *sys, int search_limit, int *ex_amount);

/* Utility: serialize result to JSON (buffer must be provided). */
int format_change_json(const CoinSystem *sys, int amount, const int *counts,
                       const char *strategy, const char *version, char *buf,
                       size_t buflen);

/* Compute total mass (grams) for counts; returns negative if mass data missing
 */
double total_mass(const CoinSystem *sys, const int *counts);
double total_diameter(const CoinSystem *sys, const int *counts);

/* Optimization mode */
typedef enum { OPT_COUNT = 0, OPT_MASS = 1, OPT_DIAMETER = 2 } OptimizeMode;

/* DP with optimization mode (count, mass, diameter). Returns 0 success. */
int dp_make_change_opt(const CoinSystem *sys, int amount, int *counts,
                       OptimizeMode mode);

#ifdef __cplusplus
}
#endif

#endif /* COINS_H */
