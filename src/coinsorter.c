/** \file coinsorter.c
 *  \brief Command-line interface for coin change algorithms and audits.
 */
#include "coins.h"
#include "color.h"
#include "version.h"
#include <ctype.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Coin system definitions moved to coin_systems.c */

/* Algorithms implemented in coin_algorithms.c */

/* ---------------- Utility ---------------- */
/** Parse non-negative int, return -1 on error. */
static int parse_int(const char *s) {
  char *e;
  long v = strtol(s, &e, 10);
  if (*e != '\0' || v < 0 || v > INT_MAX)
    return -1;
  return (int)v;
}

/** Print usage summary. */
static void print_usage(const char *prog) {
  printf("Usage: %s [amount] [system] [--json] [--audit] [--selftest] "
         "[--version] [--opt=count|mass|diam|area]\n",
         prog);
  list_systems();
}

/* Self tests: verify greedy vs DP for predefined systems and sample amounts */
/** Internal self test (greedy vs DP and canonical audit). */
static int selftest(void) {
  const CoinSystem *usd = get_coin_system("usd");
  const int AMTS[] = {0, 1, 6, 11, 37, 99, 137, 499};
  int countsA[16], countsB[16];
  for (size_t i = 0; i < sizeof(AMTS) / sizeof(AMTS[0]); ++i) {
    greedy_make_change(usd, AMTS[i], countsA);
    dp_make_change(usd, AMTS[i], countsB);
    for (size_t c = 0; c < usd->ncoins; ++c) {
      if (countsA[c] != countsB[c]) {
        fprintf(stderr, "Selftest fail amount %d coin %zu\n", AMTS[i], c);
        return 0;
      }
    }
  }
  /* canonical audit */
  int ex = 0;
  if (!audit_canonical(usd, 0, &ex)) {
    fprintf(stderr, "Unexpected non-canonical ex=%d\n", ex);
    return 0;
  }
  fprintf(stderr, "Selftest OK.\n");
  return 1;
}

/** Program entry point handling argument parsing and dispatch. */
int main(int argc, char **argv) {
  const CoinSystem *sys = get_coin_system("usd");
  int amount = -1;
  int json = 0;
  int audit = 0;
  int show_version = 0;
  OptimizeMode opt_mode = OPT_COUNT;
  int bench = 0;
  int bench_amt = 0;
  int bench_iters = 0;

  int force_no_color = 0;
  for (int i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "--json") == 0)
      json = 1;
    else if (strcmp(argv[i], "--audit") == 0)
      audit = 1;
    else if (strcmp(argv[i], "--selftest") == 0) {
      return selftest() ? 0 : 1;
    } else if (strcmp(argv[i], "--version") == 0) {
      show_version = 1;
    } else if (strncmp(argv[i], "--bench-change", 14) == 0) {
      if (i + 2 < argc) {
        bench = 1;
        bench_amt = parse_int(argv[++i]);
        bench_iters = parse_int(argv[++i]);
      } else {
        fprintf(stderr, "--bench-change requires amt iters\n");
        return 1;
      }
    } else if (strncmp(argv[i], "--opt=", 6) == 0) {
      const char *m = argv[i] + 6;
      if (strcmp(m, "count") == 0)
        opt_mode = OPT_COUNT;
      else if (strcmp(m, "mass") == 0)
        opt_mode = OPT_MASS;
      else if (strcmp(m, "diam") == 0)
        opt_mode = OPT_DIAMETER;
      else if (strcmp(m, "area") == 0)
        opt_mode = OPT_AREA;
      else {
        fprintf(stderr, "Unknown opt mode %s\n", m);
        return 1;
      }
    } else if (strcmp(argv[i], "--no-color") == 0) {
      force_no_color = 1;
    } else if (isdigit((unsigned char)argv[i][0])) {
      amount = parse_int(argv[i]);
      if (show_version) {
        printf("coinsorter version %s\n", COINSORTER_VERSION_STR);
      }
    } else {
      const CoinSystem *s2 = get_coin_system(argv[i]);
      if (s2)
        sys = s2;
      else {
        fprintf(stderr, "Unknown arg/system: %s\n", argv[i]);
        print_usage(argv[0]);
        return 1;
      }
    }
  }

  if (audit) {
    int ex = -1;
    int ok = audit_canonical(sys, 0, &ex);
    if (ok)
      printf("System %s appears canonical up to heuristic bound.\n",
             sys->system_name);
    else
      printf("System %s NON-canonical. First counterexample: %d\n",
             sys->system_name, ex);
    return ok ? 0 : 2;
  }

  if (amount < 0) {
    char line[64];
    printf("Enter amount in %s smallest units: ", sys->system_name);
    if (!fgets(line, sizeof(line), stdin))
      return 0;
    amount = parse_int(line);
    if (amount < 0) {
      fprintf(stderr, "Invalid amount.\n");
      return 1;
    }
  }

  color_init();
  if (force_no_color)
    color_enabled = 0;
  int *counts = (int *)calloc(sys->ncoins, sizeof(int));
  if (!counts) {
    perror("alloc");
    return 1;
  }
  int status = 0;
  int ex = -1;

  int do_greedy = sys->canonical_hint; /* start with hint */
  if (do_greedy) {                     /* spot-check audit small */
    if (!audit_canonical(sys, amount > 500 ? 500 : amount, &ex))
      do_greedy = 0;
  }

  if (opt_mode != OPT_COUNT) {
    /* force DP with optimization */
    do_greedy = 0;
    status = dp_make_change_opt(sys, amount, counts, opt_mode);
  } else if (do_greedy) {
    status = greedy_make_change(sys, amount, counts);
  } else {
    status = dp_make_change(sys, amount, counts);
  }

  if (status != 0) {
    fprintf(stderr, "Failed to make change for %d\n", amount);
    free(counts);
    return 1;
  }

  if (json) {
    char buf[768];
    const char *strategy = NULL;
    if (opt_mode == OPT_MASS)
      strategy = "dp-mass";
    else if (opt_mode == OPT_DIAMETER)
      strategy = "dp-diam";
    else if (opt_mode == OPT_AREA)
      strategy = "dp-area";
    else
      strategy = (do_greedy ? "greedy" : "dp");
    if (format_change_json(sys, amount, counts, strategy,
                           COINSORTER_VERSION_STR, buf, sizeof(buf)) == 0)
      puts(buf);
    else
      fprintf(stderr, "JSON overflow.\n");
  } else {
    printf("%sSystem:%s %s  Amount: %d\n", C_BOLD, C_RESET, sys->system_name,
           amount);
    int total_coins = 0;
    for (size_t i = 0; i < sys->ncoins; ++i)
      total_coins += counts[i];
    const char *mode_str =
        (opt_mode == OPT_MASS
             ? "dp-mass"
             : (opt_mode == OPT_DIAMETER
                    ? "dp-diam"
                    : (opt_mode == OPT_AREA ? "dp-area"
                                            : (do_greedy ? "greedy" : "dp"))));
    printf("Strategy: %s%s%s\n", (opt_mode == OPT_COUNT ? C_GREEN : C_MAGENTA),
           mode_str, C_RESET);
    for (size_t i = 0; i < sys->ncoins; ++i) {
      if (counts[i])
        printf("  %s (%d): %d\n", sys->coins[i].name, sys->coins[i].value,
               counts[i]);
    }
    printf("Total coins: %d\n", total_coins);
    double mass = total_mass(sys, counts);
    if (mass > 0)
      printf("Total mass: %s%.3f g%s\n", C_BLUE, mass, C_RESET);
    if (opt_mode == OPT_COUNT && !do_greedy) {
      if (ex >= 0)
        printf("(Greedy suboptimal first at amount %d)\n", ex);
      else
        printf("(Greedy optimal on sampled range)\n");
    }
  }

  if (bench) {
    if (bench_amt <= 0 || bench_iters <= 0) {
      fprintf(stderr, "Invalid bench args\n");
      return 1;
    }
    int *tmp = (int *)calloc(sys->ncoins, sizeof(int));
    if (!tmp) {
      perror("alloc");
      return 1;
    }
    clock_t t0, t1;
    double best = 1e9, tot = 0;
    for (int it = 0; it < bench_iters; ++it) {
      memset(tmp, 0, sys->ncoins * sizeof(int));
      t0 = clock();
      if (opt_mode == OPT_COUNT)
        dp_make_change(sys, bench_amt, tmp);
      else
        dp_make_change_opt(sys, bench_amt, tmp, opt_mode);
      t1 = clock();
      double dt = (double)(t1 - t0) / CLOCKS_PER_SEC;
      tot += dt;
      if (dt < best)
        best = dt;
    }
    const char *bench_mode =
        (opt_mode == OPT_COUNT
             ? "count"
             : (opt_mode == OPT_MASS
                    ? "mass"
                    : (opt_mode == OPT_DIAMETER ? "diam" : "area")));
    printf("BENCH amount=%d mode=%s iters=%d avg=%.6g s best=%.6g s\n",
           bench_amt, bench_mode, bench_iters, tot / bench_iters, best);
    free(tmp);
    return 0;
  }
  free(counts);
  return 0;
}
