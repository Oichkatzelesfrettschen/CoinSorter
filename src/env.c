#include "env.h"
#include <stdio.h>
#include <string.h>

static const Environment ENVS[] = {
    {"earth", 9.80665, 293.0, 101.325},
    {"moon", 1.625, 250.0, 0.0003},
    {"mars", 3.711, 210.0, 0.6},
    {"orbit", 0.0, 295.0, 0.0}, /* microgravity (idealized) */
};

const Environment *get_environment(const char *name) {
  if (!name)
    return &ENVS[0];
  for (size_t i = 0; i < sizeof(ENVS) / sizeof(ENVS[0]); ++i) {
    if (strcmp(ENVS[i].name, name) == 0)
      return &ENVS[i];
  }
  return NULL;
}

void list_environments(void) {
  printf("Environments:\n");
  for (size_t i = 0; i < sizeof(ENVS) / sizeof(ENVS[0]); ++i) {
    printf("  %s (g=%.3f m/s^2)\n", ENVS[i].name, ENVS[i].g);
  }
}
