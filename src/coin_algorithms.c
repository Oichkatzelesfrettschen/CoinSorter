/**
 * \file coin_algorithms.c
 * \brief Implementations for coin change algorithms, objective-weighted DP,
 * canonical audit, and JSON formatting.
 */
#include "coins.h"
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---------------- Algorithms Implementation ---------------- */

/** \brief Greedy change-making (descending coin order). */
int greedy_make_change(const CoinSystem *sys, int amount, int *counts) {
  for (size_t i = 0; i < sys->ncoins; ++i) {
    counts[i] = amount / sys->coins[i].value;
    amount -= counts[i] * sys->coins[i].value;
  }
  return amount == 0 ? 0 : -1;
}

/** \brief Optimal (minimum coin count) dynamic programming solver. */
int dp_make_change(const CoinSystem *sys, int amount, int *counts) {
  if (amount < 0)
    return -1;
  int maxC = amount + 1;
  int *best = (int *)malloc((amount + 1) * sizeof(int));
  unsigned short *last =
      (unsigned short *)malloc((amount + 1) * sizeof(unsigned short));
  if (!best || !last) {
    free(best);
    free(last);
    return -1;
  }
  for (int a = 0; a <= amount; ++a) {
    best[a] = maxC;
    last[a] = USHRT_MAX;
  }
  best[0] = 0;
  for (int a = 1; a <= amount; ++a) {
    for (size_t i = 0; i < sys->ncoins; ++i) {
      int v = sys->coins[i].value;
      if (v <= a && best[a - v] + 1 < best[a]) {
        best[a] = best[a - v] + 1;
        last[a] = (unsigned short)i;
      }
    }
  }
  if (best[amount] >= maxC) {
    free(best);
    free(last);
    return -1;
  }
  memset(counts, 0, sys->ncoins * sizeof(int));
  for (int a = amount; a > 0;) {
    unsigned short idx = last[a];
    if (idx == USHRT_MAX)
      break;
    counts[idx]++;
    a -= sys->coins[idx].value;
  }
  free(best);
  free(last);
  return 0;
}

/** \brief Test if system is canonical (greedy == optimal up to bound).
 *  \param search_limit Optional limit; if 0 derive from product of top two
 * values.
 */
int audit_canonical(const CoinSystem *sys, int search_limit, int *ex_amount) {
  if (search_limit < 0)
    search_limit = 0;
  int limit = search_limit;
  if (limit == 0) {
    int max1 = sys->coins[0].value;
    int max2 = sys->ncoins > 1 ? sys->coins[1].value : max1;
    limit = max1 * max2;
  }
  int *gcounts = (int *)malloc(sys->ncoins * sizeof(int));
  int *dcounts = (int *)malloc(sys->ncoins * sizeof(int));
  if (!gcounts || !dcounts) {
    free(gcounts);
    free(dcounts);
    return 0;
  }
  for (int amt = 1; amt <= limit; ++amt) {
    greedy_make_change(sys, amt, gcounts);
    dp_make_change(sys, amt, dcounts);
    int gc = 0, dc = 0;
    for (size_t i = 0; i < sys->ncoins; ++i) {
      gc += gcounts[i];
      dc += dcounts[i];
    }
    if (gc > dc) {
      if (ex_amount)
        *ex_amount = amt;
      free(gcounts);
      free(dcounts);
      return 0;
    }
  }
  free(gcounts);
  free(dcounts);
  return 1;
}

/** \brief Serialize change result to compact JSON buffer.
 *  Includes aggregate mass/diameter and objective meta.
 */
int format_change_json(const CoinSystem *sys, int amount, const int *counts,
                       const char *strategy, const char *version, char *buf,
                       size_t buflen) {
  double mass = total_mass(sys, counts);
  double diam = total_diameter(sys, counts);
  int total_coins = 0;
  for (size_t i = 0; i < sys->ncoins; ++i)
    total_coins += counts[i];
  const char *objective = "count";
  double objective_value = (double)total_coins;
  if (strategy) {
    if (strncmp(strategy, "dp-mass", 7) == 0) {
      objective = "mass";
      objective_value = (mass > 0 ? mass : 0.0);
    } else if (strncmp(strategy, "dp-diam", 7) == 0) {
      objective = "diameter";
      objective_value = (diam > 0 ? diam : 0.0);
    } else if (strncmp(strategy, "dp-area", 7) == 0) {
      objective = "area";
      double area = total_area(sys, counts);
      objective_value = (area > 0 ? area : 0.0);
    }
  }
  int used = snprintf(
      buf, buflen,
      "{\"system\":\"%s\",\"amount\":%d,\"strategy\":\"%s\",\"version\":\"%s\","
      "\"mass_g\":%.4f,\"diameter_mm\":%.4f,\"total_coins\":%d,\"objective\":"
      "\"%s\",\"objective_value\":%.6g,\"coins\":[",
      sys->system_name, amount, strategy ? strategy : "",
      version ? version : "", mass > 0 ? mass : 0.0, diam > 0 ? diam : 0.0,
      total_coins, objective, objective_value);
  if (used < 0 || (size_t)used >= buflen)
    return -1;
  size_t pos = used;
  for (size_t i = 0; i < sys->ncoins; ++i) {
    int n = snprintf(buf + pos, buflen - pos,
                     "%s{\"code\":\"%s\",\"value\":%d,\"count\":%d}",
                     (i ? "," : ""), sys->coins[i].code, sys->coins[i].value,
                     counts[i]);
    if (n < 0 || pos + (size_t)n >= buflen)
      return -1;
    pos += n;
  }
  int n = snprintf(buf + pos, buflen - pos, "]}");
  if (n < 0 || pos + (size_t)n >= buflen)
    return -1;
  return 0;
}

/** \brief Compute total mass (g) or -1 if any mass unknown. */
double total_mass(const CoinSystem *sys, const int *counts) {
  double mass = 0.0;
  int have = 0;
  for (size_t i = 0; i < sys->ncoins; ++i) {
    if (sys->coins[i].mass_g > 0) {
      mass += sys->coins[i].mass_g * counts[i];
      have = 1;
    }
  }
  return have ? mass : -1.0;
}

/** \brief Compute total diameter sum (mm) or -1 if any diameter unknown. */
double total_diameter(const CoinSystem *sys, const int *counts) {
  double sum = 0.0;
  int have = 0;
  for (size_t i = 0; i < sys->ncoins; ++i) {
    if (sys->coins[i].diameter_mm > 0) {
      sum += sys->coins[i].diameter_mm * counts[i];
      have = 1;
    }
  }
  return have ? sum : -1.0;
}

/** \brief Compute total planar area (mm^2) or -1 if diameter metadata missing.
 */
double total_area(const CoinSystem *sys, const int *counts) {
  double sum = 0.0;
  int have = 0;
  for (size_t i = 0; i < sys->ncoins; ++i) {
    double d = sys->coins[i].diameter_mm;
    if (d > 0) {
      double r = 0.5 * d;
      double a = M_PI * r * r;
      sum += a * counts[i];
      have = 1;
    }
  }
  return have ? sum : -1.0;
}

/** \brief Multi-objective DP optimizing weighted sum (mass or diameter) with
 * coin-count tie break. Fallback weights of 1 if metadata missing.
 */
int dp_make_change_opt(const CoinSystem *sys, int amount, int *counts,
                       OptimizeMode mode) {
  if (mode == OPT_COUNT)
    return dp_make_change(sys, amount, counts);
  typedef struct {
    double primary;
    int coins;
    int last;
  } Cell;
  Cell *dp = (Cell *)malloc((amount + 1) * sizeof(Cell));
  if (!dp)
    return -1;
  for (int a = 0; a <= amount; ++a) {
    dp[a].primary = 1e300;
    dp[a].coins = 1e9;
    dp[a].last = -1;
  }
  dp[0].primary = 0;
  dp[0].coins = 0;
  dp[0].last = -2;
  for (int a = 1; a <= amount; ++a) {
    for (size_t i = 0; i < sys->ncoins; ++i) {
      int v = sys->coins[i].value;
      if (v <= a && dp[a - v].last != -3) {
        double w;
        if (mode == OPT_MASS)
          w = sys->coins[i].mass_g;
        else if (mode == OPT_DIAMETER)
          w = sys->coins[i].diameter_mm;
        else /* OPT_AREA */ {
          double d = sys->coins[i].diameter_mm;
          w = (d > 0) ? (M_PI * 0.25 * d * d) : 0; /* area */
        }
        if (w <= 0)
          w = 1.0; /* fallback weight */
        double cand_p = dp[a - v].primary + w;
        int cand_c = dp[a - v].coins + 1;
        int better = 0;
        if (cand_p < dp[a].primary - 1e-12)
          better = 1;
        else if (fabs(cand_p - dp[a].primary) < 1e-12 && cand_c < dp[a].coins)
          better = 1;
        if (better) {
          dp[a].primary = cand_p;
          dp[a].coins = cand_c;
          dp[a].last = (int)i;
        }
      }
    }
    if (dp[a].last == -1)
      dp[a].last = -3; /* unreachable */
  }
  if (dp[amount].last < 0) {
    free(dp);
    return -1;
  }
  memset(counts, 0, sys->ncoins * sizeof(int));
  for (int a = amount; a > 0;) {
    int idx = dp[a].last;
    counts[idx]++;
    a -= sys->coins[idx].value;
  }
  free(dp);
  return 0;
}
