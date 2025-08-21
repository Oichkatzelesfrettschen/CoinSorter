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

/* Canadian Dollar (approximate; includes discontinued 1c) */
static const CoinSpec CAD_COINS[] = {
    {200, "2d", "toonie", 6.92, 28.00, "Bi-metal Ni/Al-bronze"},
    {100, "1d", "loonie", 6.27, 26.50, "Multi-ply brass plated steel"},
    {25, "25c", "quarter", 4.40, 23.88, "Multi-ply Ni plated steel"},
    {10, "10c", "dime", 1.75, 18.03, "Multi-ply Ni plated steel"},
    {5, "5c", "nickel", 3.95, 21.20, "Multi-ply Ni plated steel"},
    {1, "1c", "penny", 2.35, 19.05, "Cu plated Zn (discontinued)"}};
/* Australian Dollar */
static const CoinSpec AUD_COINS[] = {
    {200, "2d", "two dollar", 6.60, 20.50, "Al bronze"},
    {100, "1d", "one dollar", 9.00, 25.00, "Al bronze"},
    {50, "50c", "fifty cent", 15.55, 31.65, "Cupronickel"},
    {20, "20c", "twenty cent", 11.30, 28.65, "Cupronickel"},
    {10, "10c", "ten cent", 5.65, 23.60, "Cupronickel"},
    {5, "5c", "five cent", 2.83, 19.41, "Cupronickel"}};
/* New Zealand Dollar (post-2006 reduced size) */
static const CoinSpec NZD_COINS[] = {
    {200, "2d", "two dollar", 10.00, 26.50, "Al bronze"},
    {100, "1d", "one dollar", 8.00, 23.00, "Al bronze"},
    {50, "50c", "fifty cent", 5.00, 24.75, "Ni plated steel"},
    {20, "20c", "twenty cent", 4.00, 21.75, "Ni plated steel"},
    {10, "10c", "ten cent", 3.30, 20.50, "Cu plated steel"}};
/* Chinese Yuan Renminbi (current circulating) */
static const CoinSpec CNY_COINS[] = {
    {100, "1y", "1 yuan", 6.10, 25.00, "Ni plated steel"},
    {50, "5j", "5 jiao", 3.80, 20.50, "Brass alloy"},
    {10, "1j", "1 jiao", 1.15, 19.00, "Aluminum"}};

static const CoinSystem SYSTEMS[] = {
    {"usd", USD_COINS, sizeof(USD_COINS) / sizeof(USD_COINS[0]), 1, 1},
    {"eur", EUR_COINS, sizeof(EUR_COINS) / sizeof(EUR_COINS[0]), 1, 0},
    {"cad", CAD_COINS, sizeof(CAD_COINS) / sizeof(CAD_COINS[0]), 1, 0},
    {"aud", AUD_COINS, sizeof(AUD_COINS) / sizeof(AUD_COINS[0]), 1, 0},
    {"nzd", NZD_COINS, sizeof(NZD_COINS) / sizeof(NZD_COINS[0]), 1, 0},
    {"cny", CNY_COINS, sizeof(CNY_COINS) / sizeof(CNY_COINS[0]), 1, 0}};

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
