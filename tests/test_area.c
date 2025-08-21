#include "coins.h"
#include <math.h>
#include <stdio.h>

int main(void) {
  const CoinSystem *usd = get_coin_system("usd");
  if (!usd) {
    fprintf(stderr, "usd missing\n");
    return 1;
  }
  int counts[32] = {0};
  int amount = 137;
  if (dp_make_change_opt(usd, amount, counts, OPT_AREA) != 0) {
    fprintf(stderr, "dp area fail\n");
    return 1;
  }
  int value = 0;
  int totalc = 0;
  for (size_t i = 0; i < usd->ncoins; ++i) {
    value += counts[i] * usd->coins[i].value;
    totalc += counts[i];
  }
  if (value != amount) {
    fprintf(stderr, "value mismatch got %d\n", value);
    return 1;
  }
  double area = total_area(usd, counts);
  if (area <= 0) {
    fprintf(stderr, "area non-positive %.3f\n", area);
    return 1;
  }
  /* simple regression: recompute area by manual formula */
  double area2 = 0.0;
  for (size_t i = 0; i < usd->ncoins; ++i) {
    double d = usd->coins[i].diameter_mm;
    if (d > 0 && counts[i] > 0) {
      double r = 0.5 * d;
      area2 += M_PI * r * r * counts[i];
    }
  }
  if (fabs(area - area2) > 1e-9) {
    fprintf(stderr, "area mismatch %.10f vs %.10f\n", area, area2);
    return 1;
  }
  printf("area test passed (coins=%d area=%.4f)\n", totalc, area);
  return 0;
}
