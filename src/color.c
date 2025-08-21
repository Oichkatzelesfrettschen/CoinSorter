/** \file color.c
 *  \brief ANSI color enable / disable initialization logic.
 */
#include "color.h"
#include <stdio.h>
#include <stdlib.h>
#ifdef _WIN32
#include <io.h>
#define isatty _isatty
#define fileno _fileno
#else
#include <unistd.h>
#endif

int color_enabled = 0; /**< Global flag after initialization. */
/** Initialize color_enabled considering NO_COLOR / FORCE_COLOR / TTY. */
void color_init(void) {
  if (getenv("NO_COLOR")) {
    color_enabled = 0;
    return;
  }
  const char *force = getenv("FORCE_COLOR");
  if (force && *force) {
    color_enabled = 1;
    return;
  }
  if (isatty(fileno(stdout)))
    color_enabled = 1;
  else
    color_enabled = 0;
}
