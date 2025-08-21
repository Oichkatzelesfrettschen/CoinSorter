/** Simple portable ANSI color helpers (auto-disabled if not a TTY or NO_COLOR set) */
#ifndef COLOR_H
#define COLOR_H

#ifdef __cplusplus
extern "C" {
#endif

extern int color_enabled; /* 0 or 1 */
void color_init(void);    /* call early */

#define C_RESET   (color_enabled?"\x1b[0m":"")
#define C_BOLD    (color_enabled?"\x1b[1m":"")
#define C_DIM     (color_enabled?"\x1b[2m":"")
#define C_RED     (color_enabled?"\x1b[31m":"")
#define C_GREEN   (color_enabled?"\x1b[32m":"")
#define C_YELLOW  (color_enabled?"\x1b[33m":"")
#define C_BLUE    (color_enabled?"\x1b[34m":"")
#define C_MAGENTA (color_enabled?"\x1b[35m":"")
#define C_CYAN    (color_enabled?"\x1b[36m":"")
#define C_GRAY    (color_enabled?"\x1b[90m":"")

#ifdef __cplusplus
}
#endif

#endif /* COLOR_H */
