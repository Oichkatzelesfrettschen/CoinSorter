#include "coins.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int sum_value(const CoinSystem *s, const int *c) {
  int total = 0;
  for (size_t i = 0; i < s->ncoins; i++)
    total += c[i] * s->coins[i].value;
  return total;
}
static int sum_counts(const CoinSystem *s, const int *c) {
  int total = 0;
  for (size_t i = 0; i < s->ncoins; i++)
    total += c[i];
  return total;
}

int main(void) {
  const CoinSystem *usd = get_coin_system("usd");
  if (!usd) {
    fprintf(stderr, "usd system missing\n");
    return 1;
  }
  const CoinSystem *eur = get_coin_system("eur");
  if (!eur) {
    fprintf(stderr, "eur system missing\n");
    return 1;
  }
  /* unknown system should be NULL */
  if (get_coin_system("nope") != NULL) {
    fprintf(stderr, "unknown system unexpectedly found\n");
    return 1;
  }

  int amt = 137;
  int c1[32] = {0}, c2[32] = {0};
  if (dp_make_change(usd, amt, c1) != 0) {
    fprintf(stderr, "dp_make_change failed\n");
    return 1;
  }
  if (dp_make_change_opt(usd, amt, c2, OPT_COUNT) != 0) {
    fprintf(stderr, "dp_make_change_opt(count) failed\n");
    return 1;
  }
  for (size_t i = 0; i < usd->ncoins; i++)
    if (c1[i] != c2[i]) {
      fprintf(stderr, "count mode mismatch\n");
      return 1;
    }

  /* mass vs diameter objective differentiation */
  int cm[32] = {0}, cd[32] = {0};
  if (dp_make_change_opt(usd, amt, cm, OPT_MASS) != 0) {
    fprintf(stderr, "mass opt fail\n");
    return 1;
  }
  if (dp_make_change_opt(usd, amt, cd, OPT_DIAMETER) != 0) {
    fprintf(stderr, "diam opt fail\n");
    return 1;
  }
  int ca[32] = {0};
  if (dp_make_change_opt(usd, amt, ca, OPT_AREA) != 0) {
    fprintf(stderr, "area opt fail\n");
    return 1;
  }
  if (sum_value(usd, cm) != amt || sum_value(usd, cd) != amt) {
    fprintf(stderr, "value mismatch in objectives\n");
    return 1;
  }
  double mass_m = total_mass(usd, cm);
  double mass_d = total_mass(usd, cd);
  double diam_m = total_diameter(usd, cm);
  double diam_d = total_diameter(usd, cd);
  if (!(fabs(mass_m - mass_d) > 1e-9 || fabs(diam_m - diam_d) > 1e-9 ||
        sum_counts(usd, cm) != sum_counts(usd, cd))) {
    /* Not strictly required they differ but likely. Warn only */
    fprintf(stderr, "warning: mass/diam solutions identical\n");
  }

  /* JSON serialization tests */
  char buf[2048];
  if (format_change_json(usd, amt, cm, "dp-mass", "X", buf, sizeof(buf)) != 0) {
    fprintf(stderr, "json mass fail\n");
    return 1;
  }
  if (!strstr(buf, "\"objective\":\"mass\"")) {
    fprintf(stderr, "json missing objective mass\n");
    return 1;
  }
  if (!strstr(buf, "\"total_coins\"")) {
    fprintf(stderr, "json missing total_coins\n");
    return 1;
  }
  if (format_change_json(usd, amt, cd, "dp-diam", "X", buf, sizeof(buf)) != 0) {
    fprintf(stderr, "json diam fail\n");
    return 1;
  }
  if (!strstr(buf, "\"objective\":\"diameter\"")) {
    fprintf(stderr, "json missing objective diameter\n");
    return 1;
  }
  if (format_change_json(usd, amt, ca, "dp-area", "X", buf, sizeof(buf)) != 0) {
    fprintf(stderr, "json area fail\n");
    return 1;
  }
  if (!strstr(buf, "\"objective\":\"area\"")) {
    fprintf(stderr, "json missing objective area\n");
    return 1;
  }

  /* small buffer failure */
  char tiny[32];
  if (format_change_json(usd, amt, cm, "dp", "X", tiny, sizeof(tiny)) == 0) {
    fprintf(stderr, "expected failure with tiny buffer\n");
    return 1;
  }

  /* Canonical audit: usd should be canonical */
  int ex = 0;
  if (!audit_canonical(usd, 0, &ex)) {
    fprintf(stderr, "usd canonical audit failed ex=%d\n", ex);
    return 1;
  }
  /* eur may or may not be canonical; just call to exercise path */
  audit_canonical(eur, 0, &ex);

  printf("advanced coin tests passed\n");
  return 0;
}
