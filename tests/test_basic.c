#include "beta.h"
#include "coins.h"
#include <math.h>
#include <stdio.h>

int main(void) {
  double b1 = beta1();
  double b2 = beta2();
  if (fabs(b1 - 3.0 / (16.0 * M_PI * M_PI)) > 1e-12) {
    fprintf(stderr, "beta1 mismatch\n");
    return 1;
  }
  if (fabs(b2 + 17.0 / (1536.0 * pow(M_PI, 4))) > 1e-12) {
    fprintf(stderr, "beta2 mismatch\n");
    return 1;
  }
  const CoinSystem *usd = get_coin_system("usd");
  int counts[8];
  greedy_make_change(usd, 137, counts);
  int total = 0;
  for (size_t i = 0; i < usd->ncoins; ++i)
    total += counts[i] * usd->coins[i].value;
  if (total != 137) {
    fprintf(stderr, "greedy sum mismatch\n");
    return 1;
  }
  printf("basic tests passed\n");
  return 0;
}
