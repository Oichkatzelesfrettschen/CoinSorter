/** \file superforce_ncui.c
 *  \brief Ncurses UI: coin optimization + simulations + physics overlays.
 */
#include "beta.h"
#include "casimir.h"
#include "coins.h"
#include "env.h"
#include "observables.h"
#include "simulation.h"
#include "version.h"
#include <curses.h>
#include <locale.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct {
  int x0, x1;
  int action;
} ClickRegion;

enum {
  ACT_AMT_INC,
  ACT_AMT_DEC,
  ACT_OPT,
  ACT_SYS,
  ACT_ENV,
  ACT_FBM,
  ACT_VALUE,
  ACT_SIZE_UP,
  ACT_SIZE_DOWN,
  ACT_H_UP,
  ACT_H_DOWN,
  ACT_SOLVE,
  ACT_JSON,
  ACT_POISSON,
  ACT_VECTORS,
  ACT_MLP,
  ACT_PHYS,
  ACT_NONE
};

/** \brief Application state container for ncurses UI. */
typedef struct {
  const CoinSystem *sys;             /**< Active coin system. */
  int amount;                        /**< Change amount in smallest units. */
  OptimizeMode opt;                  /**< Active optimization objective. */
  const Environment *env;            /**< Active environment (gravity etc.). */
  int thermal;                       /**< Thermal Casimir toggle. */
  int fbm_size;                      /**< Field size (square). */
  double fbm_H;                      /**< Hurst exponent for fractal gen. */
  double *fbm;                       /**< Height / value noise field. */
  double *phi;                       /**< Poisson potential solution. */
  double *dx;                        /**< Gradient x component. */
  double *dy;                        /**< Gradient y component. */
  double energy_avg;                 /**< Average energy density over field. */
  double last_resid;                 /**< Last Poisson residual. */
  int focus;                         /**< (Reserved for future input focus). */
  int counts[32];                    /**< Change counts workspace. */
  ClickRegion regions[24];           /**< Clickable button regions. */
  int nregions;                      /**< Number of active regions. */
  int show_physics;                  /**< Physics overlay toggle. */
  int show_vectors;                  /**< Vector arrow overlay toggle. */
  int show_residual;                 /**< Residual heatmap toggle. */
  double beta1_v, beta2_v, gamma1_v; /**< Special function values. */
  double cas_vals[4];                /**< Casimir force samples. */
} App;

static void free_sim(App *A) {
  free(A->fbm);
  free(A->phi);
  free(A->dx);
  free(A->dy);
  A->fbm = A->phi = A->dx = A->dy = NULL;
}

static void app_init(App *A) {
  memset(A, 0, sizeof(*A));
  A->sys = get_coin_system("usd");
  A->amount = 137;
  A->opt = OPT_COUNT;
  A->env = get_environment("earth");
  A->thermal = 1;
  A->fbm_size = 65;
  A->fbm_H = 0.6;
  A->last_resid = -1.0;
}

static void generate_fbm_nc(App *A) {
  free(A->fbm);
  A->fbm = NULL;
  int N = A->fbm_size;
  A->fbm = (double *)malloc(sizeof(double) * N * N);
  if (!A->fbm)
    return;
  if (fbm_diamond_square(A->fbm, N, A->fbm_H, 0) != 0)
    generate_fbm(A->fbm, N, N, A->fbm_H);
}

static const char *opt_label(OptimizeMode m) {
  switch (m) {
  case OPT_COUNT:
    return "count";
    break;
  case OPT_MASS:
    return "mass";
    break;
  case OPT_DIAMETER:
    return "diam";
    break;
  case OPT_AREA:
    return "area";
    break;
  default:
    return "?";
  }
}

static const char *strategy_name(const App *A) {
  switch (A->opt) {
  case OPT_MASS:
    return "dp-mass";
    break;
  case OPT_DIAMETER:
    return "dp-diam";
    break;
  case OPT_AREA:
    return "dp-area";
    break;
  default:
    return "dp";
  }
}

/** \brief Draw status / buttons bar and dynamic metrics. */
static void draw_status(WINDOW *w, App *A) {
  werase(w);
  box(w, 0, 0);
  A->nregions = 0;
  int x = 2, y = 1;
  mvwprintw(w, y, x, "%s", A->sys->system_name);
  x += (int)strlen(A->sys->system_name) + 1;
#define ADD_BTN(lbl, act)                                                      \
  do {                                                                         \
    int l = (int)strlen(lbl);                                                  \
    mvwprintw(w, y, x, "[%s]", lbl);                                           \
    A->regions[A->nregions].x0 = x;                                            \
    A->regions[A->nregions].x1 = x + l + 1;                                    \
    A->regions[A->nregions].action = act;                                      \
    if (A->nregions < (int)(sizeof(A->regions) / sizeof(A->regions[0])) - 1)   \
      A->nregions++;                                                           \
    x += l + 3;                                                                \
  } while (0)
  ADD_BTN("+", ACT_AMT_INC);
  ADD_BTN("-", ACT_AMT_DEC);
  ADD_BTN("opt", ACT_OPT);
  ADD_BTN("sys", ACT_SYS);
  ADD_BTN("env", ACT_ENV);
  ADD_BTN("fbm", ACT_FBM);
  ADD_BTN("val", ACT_VALUE);
  ADD_BTN("sz+", ACT_SIZE_UP);
  ADD_BTN("sz-", ACT_SIZE_DOWN);
  ADD_BTN("H+", ACT_H_UP);
  ADD_BTN("H-", ACT_H_DOWN);
  ADD_BTN("solve", ACT_SOLVE);
  ADD_BTN("json", ACT_JSON);
  ADD_BTN("pois", ACT_POISSON);
  ADD_BTN("vec", ACT_VECTORS);
  ADD_BTN("mlp", ACT_MLP);
  ADD_BTN("phys", ACT_PHYS);
#undef ADD_BTN
  mvwprintw(w, y + 1, 2,
            "amt=%d opt=%s env=%s g=%.2f H=%.2f N=%d resid=%+.2g E=%.3g%s%s",
            A->amount, opt_label(A->opt), A->env->name, A->env->g, A->fbm_H,
            A->fbm_size, A->last_resid, A->energy_avg,
            A->show_vectors ? " v" : "", A->show_residual ? " r" : "");
  wrefresh(w);
}

static void draw_help(WINDOW *w) {
  werase(w);
  box(w, 0, 0);
  int r = 1;
  mvwprintw(w, r++, 2,
            "Keys: q quit + - amt o opt y system e env f fbm n value noise");
  mvwprintw(
      w, r++, 2,
      "       s/S size h/H Hurst c solve j json p poisson v vectors r resid");
  mvwprintw(w, r++, 2, "       g physics m MLP");
  mvwprintw(w, r++, 2, "Mouse: click buttons in status bar");
  mvwprintw(w, r++, 2, "Energy avg shown when vectors computed (vec)");
  mvwprintw(w, r++, 2, "JSON -> ncui_change.json");
  wrefresh(w);
}

static void solve_change(App *A, WINDOW *w) {
  memset(A->counts, 0, sizeof(A->counts));
  if (A->opt == OPT_COUNT)
    dp_make_change(A->sys, A->amount, A->counts);
  else
    dp_make_change_opt(A->sys, A->amount, A->counts, A->opt);
  werase(w);
  box(w, 0, 0);
  int row = 1;
  mvwprintw(w, row++, 2, "Change %d (%s)", A->amount, opt_label(A->opt));
  int tot = 0;
  for (size_t i = 0; i < A->sys->ncoins && row < getmaxy(w) - 2; ++i)
    if (A->counts[i]) {
      mvwprintw(w, row++, 2, "%s x %d", A->sys->coins[i].code, A->counts[i]);
      tot += A->counts[i];
    }
  mvwprintw(w, row, 2, "Total:%d", tot);
  wrefresh(w);
}

static void export_json(const App *A) {
  char buf[2048];
  if (format_change_json(A->sys, A->amount, A->counts, strategy_name(A),
                         COINSORTER_VERSION_STR, buf, sizeof(buf)) == 0) {
    FILE *fp = fopen("ncui_change.json", "w");
    if (fp) {
      fputs(buf, fp);
      fclose(fp);
    }
  }
}

static void compute_physics(App *A) {
  A->beta1_v = beta1();
  A->beta2_v = beta2();
  A->gamma1_v = gamma_phi(1.0);
  double R = 5e-6, d = 10e-9, T = 4.0, ani = 5.0;
  double F0 = casimir_base(R, d);
  double Fth = A->thermal ? casimir_thermal(R, d, T) : 0.0;
  for (int i = 0; i < 4; ++i) {
    double th = (i * 60) * M_PI / 180.0;
    A->cas_vals[i] = casimir_modulated(F0, Fth, ani, th);
  }
}

static void run_poisson(App *A, WINDOW *w_sim) {
  if (!A->fbm)
    return;
  int N = A->fbm_size;
  int NN = N * N;
  free(A->phi);
  A->phi = (double *)calloc(NN, sizeof(double));
  if (!A->phi)
    return;
  double *rhs = (double *)calloc(NN, sizeof(double));
  if (!rhs)
    return;
  for (int y = 1; y < N - 1; ++y)
    for (int x = 1; x < N - 1; ++x) {
      int i = y * N + x;
      rhs[i] = 4 * A->fbm[i] - A->fbm[i - 1] - A->fbm[i + 1] - A->fbm[i - N] -
               A->fbm[i + N];
    }
  int iters = 200, step = 20;
  for (int done = 0; done < iters; done += step) {
    A->last_resid = poisson_jacobi(A->phi, rhs, N, N, step);
    werase(w_sim);
    box(w_sim, 0, 0);
    mvwprintw(w_sim, 1, 2, "Poisson %d/%d resid %.3g", done + step, iters,
              A->last_resid);
    wrefresh(w_sim);
    napms(40);
  }
  free(rhs);
}

static void run_vectors(App *A) {
  if (!A->fbm)
    return;
  int N = A->fbm_size;
  int NN = N * N;
  free(A->dx);
  free(A->dy);
  A->dx = (double *)malloc(sizeof(double) * NN);
  A->dy = (double *)malloc(sizeof(double) * NN);
  if (!A->dx || !A->dy) {
    free(A->dx);
    free(A->dy);
    A->dx = A->dy = NULL;
    return;
  }
  compute_deflection(A->fbm, N, N, A->dx, A->dy);
  double acc = 0.0;
  int samples = 0;
  for (int y = 1; y < N - 1; ++y)
    for (int x = 1; x < N - 1; ++x) {
      int i = y * N + x;
      acc += observable_energy_density(A->dx[i], A->dy[i]);
      ++samples;
    }
  A->energy_avg = samples ? acc / samples : 0.0;
}

/** \brief Render simulation field plus optional vector/residual overlays. */
static void draw_sim(WINDOW *w, const App *A) {
  werase(w);
  box(w, 0, 0);
  int H, W;
  getmaxyx(w, H, W);
  if (!A->fbm) {
    mvwprintw(w, 1, 2, "<no field>");
    wrefresh(w);
    return;
  }
  int N = A->fbm_size;
  int uh = H - 3;
  int uw = W - 2;
  if (uh < 1 || uw < 1) {
    wrefresh(w);
    return;
  }
  const char *lut = " .:-=+*#%@";
  /* Pre-compute Poisson residual magnitudes if heatmap enabled */
  double resid_min = 1e9, resid_max = -1e9;
  int heat_enabled = (A->show_residual && A->phi);
  double *resid_tmp = NULL;
  if (heat_enabled) {
    resid_tmp = (double *)malloc(sizeof(double) * N * N);
    if (resid_tmp) {
      for (int y = 1; y < N - 1; ++y) {
        for (int x = 1; x < N - 1; ++x) {
          int i = y * N + x;
          double lap = A->phi[i - 1] + A->phi[i + 1] + A->phi[i - N] +
                       A->phi[i + N] - 4 * A->phi[i];
          double r = fabs(lap);
          resid_tmp[i] = r;
          if (r < resid_min)
            resid_min = r;
          if (r > resid_max)
            resid_max = r;
        }
      }
      if (resid_max <= resid_min)
        resid_max = resid_min + 1e-9;
    }
  }
  for (int yy = 0; yy < uh; ++yy) {
    for (int xx = 0; xx < uw; ++xx) {
      int x = (int)((xx / (double)uw) * (N - 1));
      int y = (int)((yy / (double)uh) * (N - 1));
      int i = y * N + x;
      double v = A->fbm[i];
      int shade = (int)((v + 1.0) * 0.5 * 9);
      if (shade < 0)
        shade = 0;
      if (shade > 9)
        shade = 9;
      chtype ch = lut[shade];
      if (heat_enabled && resid_tmp) {
        double rr = resid_tmp[i];
        double t = (rr - resid_min) / (resid_max - resid_min);
        if (t > 1)
          t = 1;
        if (t < 0)
          t = 0;
        /* map t -> reduced palette (use color pairs if available) */
        if (has_colors()) {
          int cp = 1 + (int)(t * 4); /* 1..5 */
          wattron(w, COLOR_PAIR(cp));
          mvwaddch(w, yy + 1, xx + 1, ch);
          wattroff(w, COLOR_PAIR(cp));
          continue;
        } else {
          ch = (t > 0.66) ? '#' : (t > 0.33 ? '*' : ch);
        }
      }
      mvwaddch(w, yy + 1, xx + 1, ch);
    }
  }
  free(resid_tmp);
  if (A->dx && A->dy) {
    mvwprintw(w, H - 2, 2, "Eavg=%.3g", A->energy_avg);
    if (A->show_vectors) {
      /* Arrow overlay (subsample to avoid clutter) */
      static const char *arrows = "→↗↑↖←↙↓↘"; /* 8 directions */
      int step = (uw > 80 || uh > 40) ? 4 : 2;
      for (int yy = 0; yy < uh; yy += step) {
        for (int xx = 0; xx < uw; xx += step) {
          int x = (int)((xx / (double)uw) * (N - 1));
          int y = (int)((yy / (double)uh) * (N - 1));
          int i = y * N + x;
          double vx = A->dx[i];
          double vy = A->dy[i];
          double mag = sqrt(vx * vx + vy * vy) + 1e-12;
          double ang = atan2(vy, vx);
          double pi = M_PI;
          double sector = (ang + pi) / (2 * pi) * 8.0; /* 0..8 */
          int idx = (int)floor(sector) & 7;
          chtype ach = arrows[idx] | A_BOLD;
          /* scale brightness by magnitude (approx) using color pair */
          if (has_colors()) {
            int cp = 1 + (int)fmin(mag * 8.0, 4.0);
            wattron(w, COLOR_PAIR(cp));
            mvwaddch(w, yy + 1, xx + 1, ach);
            wattroff(w, COLOR_PAIR(cp));
          } else {
            mvwaddch(w, yy + 1, xx + 1, ach);
          }
        }
      }
    }
  }
  if (A->show_physics) {
    mvwprintw(w, 1, W - 30, "b1 %.1e b2 %.1e g %.1e", A->beta1_v, A->beta2_v,
              A->gamma1_v);
    mvwprintw(w, 2, W - 30, "Cas %.1e %.1e", A->cas_vals[0], A->cas_vals[1]);
  }
  wrefresh(w);
}

static void run_mlp_demo(WINDOW *w_change) {
  int h = getmaxy(w_change);
  MLP mlp;
  if (mlp_init(&mlp, 2, 8, 2, 123) != 0)
    return;
  int n = 20;
  double xs[40], ys[40];
  for (int i = 0; i < n; ++i) {
    double a = (i / (double)(n - 1)) * 2 - 1;
    double b = -a;
    xs[2 * i] = a;
    xs[2 * i + 1] = b;
    ys[2 * i] = a;
    ys[2 * i + 1] = b;
  }
  for (int e = 0; e <= 160; ++e) {
    mlp_train_epoch(&mlp, xs, ys, n, 0.01);
    if (e % 40 == 0) {
      double loss = 0;
      double o[2];
      for (int i = 0; i < n; ++i) {
        mlp_forward(&mlp, &xs[2 * i], o);
        double dx = o[0] - ys[2 * i];
        double dy = o[1] - ys[2 * i + 1];
        loss += dx * dx + dy * dy;
      }
      loss /= n;
      mvwprintw(w_change, h - 2, 2, "MLP e%3d loss %.3g   ", e, loss);
      wrefresh(w_change);
      napms(40);
    }
  }
  mvwprintw(w_change, h - 2, 2, "MLP done          ");
  wrefresh(w_change);
  mlp_free(&mlp);
}

int main(void) {
  setlocale(LC_ALL, "");
  initscr();
  cbreak();
  noecho();
  keypad(stdscr, TRUE);
  mousemask(ALL_MOUSE_EVENTS, NULL);
  curs_set(0);
  if (has_colors()) {
    start_color();
    use_default_colors();
    init_pair(1, COLOR_CYAN, -1);
    init_pair(2, COLOR_GREEN, -1);
    init_pair(3, COLOR_MAGENTA, -1);
    init_pair(4, COLOR_YELLOW, -1);
    init_pair(5, COLOR_BLUE, -1);
  }
  App A;
  app_init(&A);
  int H, W;
  getmaxyx(stdscr, H, W);
  int help_h = 6;
  WINDOW *w_status = newwin(3, W, 0, 0);
  WINDOW *w_help = newwin(help_h, W, H - help_h, 0);
  int mid_h = H - 3 - help_h;
  if (mid_h < 6)
    mid_h = 6;
  int left_w = W / 3;
  WINDOW *w_change = newwin(mid_h, left_w, 3, 0);
  WINDOW *w_sim = newwin(mid_h, W - left_w, 3, left_w);
  draw_status(w_status, &A);
  draw_help(w_help);
  solve_change(&A, w_change);
  draw_sim(w_sim, &A);
  MEVENT me;
  int ch;
  while ((ch = getch()) != 'q') {
    if (ch == KEY_MOUSE) {
      if (getmouse(&me) == OK && me.y == 1) {
        for (int i = 0; i < A.nregions; ++i)
          if (me.x >= A.regions[i].x0 && me.x <= A.regions[i].x1) {
            int act = A.regions[i].action;
            switch (act) {
            case ACT_AMT_INC:
              ++A.amount;
              solve_change(&A, w_change);
              break;
            case ACT_AMT_DEC:
              if (A.amount > 0)
                --A.amount;
              solve_change(&A, w_change);
              break;
            case ACT_OPT:
              A.opt = (A.opt + 1) % OPT_MODE_COUNT;
              solve_change(&A, w_change);
              break;
            case ACT_SYS:
              A.sys = (strcmp(A.sys->system_name, "usd") == 0)
                          ? get_coin_system("eur")
                          : get_coin_system("usd");
              solve_change(&A, w_change);
              break;
            case ACT_ENV:
              A.env = (strcmp(A.env->name, "earth") == 0)
                          ? get_environment("moon")
                          : (strcmp(A.env->name, "moon") == 0
                                 ? get_environment("mars")
                                 : (strcmp(A.env->name, "mars") == 0
                                        ? get_environment("orbit")
                                        : get_environment("earth")));
              break;
            case ACT_FBM:
              generate_fbm_nc(&A);
              draw_sim(w_sim, &A);
              break;
            case ACT_VALUE: {
              int N = A.fbm_size;
              free(A.fbm);
              A.fbm = (double *)malloc(sizeof(double) * N * N);
              if (A.fbm) {
                generate_value_noise(A.fbm, N, N, (unsigned)time(NULL), 4);
                draw_sim(w_sim, &A);
              }
            } break;
            case ACT_SIZE_UP: {
              int sizes[] = {17, 33, 65, 129, 257};
              for (int si = 0; si < 5; ++si)
                if (sizes[si] == A.fbm_size && si < 4) {
                  A.fbm_size = sizes[si + 1];
                  break;
                }
              generate_fbm_nc(&A);
              draw_sim(w_sim, &A);
            } break;
            case ACT_SIZE_DOWN: {
              int sizes[] = {17, 33, 65, 129, 257};
              for (int si = 0; si < 5; ++si)
                if (sizes[si] == A.fbm_size && si > 0) {
                  A.fbm_size = sizes[si - 1];
                  break;
                }
              generate_fbm_nc(&A);
              draw_sim(w_sim, &A);
            } break;
            case ACT_H_UP:
              if (A.fbm_H < 0.95)
                A.fbm_H += 0.05;
              generate_fbm_nc(&A);
              draw_sim(w_sim, &A);
              break;
            case ACT_H_DOWN:
              if (A.fbm_H > 0.05)
                A.fbm_H -= 0.05;
              generate_fbm_nc(&A);
              draw_sim(w_sim, &A);
              break;
            case ACT_SOLVE:
              solve_change(&A, w_change);
              break;
            case ACT_JSON:
              export_json(&A);
              mvwprintw(w_help, 4, 2, "JSON written");
              wrefresh(w_help);
              break;
            case ACT_POISSON:
              run_poisson(&A, w_sim);
              draw_sim(w_sim, &A);
              break;
            case ACT_VECTORS:
              if (!A.show_vectors) {
                run_vectors(&A);
                A.show_vectors = 1;
              } else {
                A.show_vectors = 0;
              }
              draw_sim(w_sim, &A);
              break;
            case ACT_MLP:
              run_mlp_demo(w_change);
              break;
            case ACT_PHYS:
              A.show_physics = !A.show_physics;
              if (A.show_physics)
                compute_physics(&A);
              draw_sim(w_sim, &A);
              break;
            default:
              break;
            }
          }
      }
    } else
      switch (ch) {
      case '+':
        ++A.amount;
        solve_change(&A, w_change);
        break;
      case '-':
        if (A.amount > 0)
          --A.amount;
        solve_change(&A, w_change);
        break;
      case 'o':
        A.opt = (A.opt + 1) % OPT_MODE_COUNT;
        solve_change(&A, w_change);
        break;
      case 'y':
        A.sys = (strcmp(A.sys->system_name, "usd") == 0)
                    ? get_coin_system("eur")
                    : get_coin_system("usd");
        solve_change(&A, w_change);
        break;
      case 'e':
        A.env = (strcmp(A.env->name, "earth") == 0)
                    ? get_environment("moon")
                    : (strcmp(A.env->name, "moon") == 0
                           ? get_environment("mars")
                           : (strcmp(A.env->name, "mars") == 0
                                  ? get_environment("orbit")
                                  : get_environment("earth")));
        break;
      case 'f':
        generate_fbm_nc(&A);
        draw_sim(w_sim, &A);
        break;
      case 'n': {
        int N = A.fbm_size;
        free(A.fbm);
        A.fbm = (double *)malloc(sizeof(double) * N * N);
        if (A.fbm) {
          generate_value_noise(A.fbm, N, N, (unsigned)time(NULL), 4);
          draw_sim(w_sim, &A);
        }
      } break;
      case 's': {
        int sizes[] = {17, 33, 65, 129, 257};
        for (int si = 0; si < 5; ++si)
          if (sizes[si] == A.fbm_size && si > 0) {
            A.fbm_size = sizes[si - 1];
            break;
          }
        generate_fbm_nc(&A);
        draw_sim(w_sim, &A);
      } break;
      case 'S': {
        int sizes[] = {17, 33, 65, 129, 257};
        for (int si = 0; si < 5; ++si)
          if (sizes[si] == A.fbm_size && si < 4) {
            A.fbm_size = sizes[si + 1];
            break;
          }
        generate_fbm_nc(&A);
        draw_sim(w_sim, &A);
      } break;
      case 'h':
        if (A.fbm_H > 0.05) {
          A.fbm_H -= 0.05;
          generate_fbm_nc(&A);
          draw_sim(w_sim, &A);
        }
        break;
      case 'H':
        if (A.fbm_H < 0.95) {
          A.fbm_H += 0.05;
          generate_fbm_nc(&A);
          draw_sim(w_sim, &A);
        }
        break;
      case 'c':
        solve_change(&A, w_change);
        break;
      case 'j':
        export_json(&A);
        mvwprintw(w_help, 4, 2, "JSON written");
        wrefresh(w_help);
        break;
      case 'p':
        run_poisson(&A, w_sim);
        draw_sim(w_sim, &A);
        break;
      case 'v':
        if (!A.show_vectors) {
          run_vectors(&A);
          A.show_vectors = 1;
        } else {
          A.show_vectors = 0;
        }
        draw_sim(w_sim, &A);
        break;
      case 'r': /* residual heatmap toggle */
        A.show_residual = !A.show_residual;
        draw_sim(w_sim, &A);
        break;
      case 'm':
        run_mlp_demo(w_change);
        break;
      case 'g':
        A.show_physics = !A.show_physics;
        if (A.show_physics)
          compute_physics(&A);
        draw_sim(w_sim, &A);
        break;
      case '?':
        draw_help(w_help);
        break;
      default:
        break;
      }
    draw_status(w_status, &A);
  }
  endwin();
  free_sim(&A);
  return 0;
}
