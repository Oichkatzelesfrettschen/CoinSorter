#include "coins.h"
#include <stdio.h>
#include <string.h>

static const CoinSpec USD_COINS[] = {
    {25, "25c", "quarter", 5.670, 24.26, "8.33% Ni bal Cu (clad)"},
    {10, "10c", "dime", 2.268, 17.91, "8.33% Ni bal Cu (clad)"},
    {5, "5c", "nickel", 5.000, 21.21, "25% Ni bal Cu"},
    {1, "1c", "penny", 2.500, 19.05, "2.5% Cu 97.5% Zn (plated)"}};

static const CoinSpec EUR_COINS[] = {
    {200, "2e", "2 euro", 8.500, 25.75, "Bi-metal: Ni brass/Cu-Ni"},
    {100, "1e", "1 euro", 7.500, 23.25, "Bi-metal: Cu-Ni/Ni brass"},
    {50, "50c", "50 cent", 7.800, 24.25, "Nordic gold"},
    {20, "20c", "20 cent", 5.740, 22.25, "Nordic gold"},
    {10, "10c", "10 cent", 4.100, 19.75, "Nordic gold"},
    {5, "5c", "5 cent", 3.920, 21.25, "Cu plated steel"},
    {2, "2c", "2 cent", 3.060, 18.75, "Cu plated steel"},
    {1, "1c", "1 cent", 2.300, 16.25, "Cu plated steel"}};

static const CoinSystem SYSTEMS[] = {
    {"usd", USD_COINS, sizeof(USD_COINS) / sizeof(USD_COINS[0]), 1, 1},
    {"eur", EUR_COINS, sizeof(EUR_COINS) / sizeof(EUR_COINS[0]), 1, 0}};

const CoinSystem *get_coin_system(const char *name) {
  if (!name)
    return NULL;
  for (size_t i = 0; i < sizeof(SYSTEMS) / sizeof(SYSTEMS[0]); ++i) {
    if (strcmp(SYSTEMS[i].system_name, name) == 0)
      return &SYSTEMS[i];
  }
  return NULL;
}

void list_systems(void) {
  printf("Available systems:\n");
  for (size_t i = 0; i < sizeof(SYSTEMS) / sizeof(SYSTEMS[0]); ++i) {
    printf("  %s (%zu coins)\n", SYSTEMS[i].system_name, SYSTEMS[i].ncoins);
  }
}
