/** \file superforce_ui.c
 *  \brief Minimal interactive (non-ncurses) UI for demonstrations.
 */
#include <ctype.h>
#include <locale.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "beta.h"
#include "casimir.h"
#include "coins.h"
#include "color.h"
#include "env.h"
#include "simulation.h"
#include "version.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

typedef struct {
  const CoinSystem *coin_sys;
  int amount;
  OptimizeMode opt_mode;
  const Environment *env;
  int thermal; /* include thermal Casimir */
  int fbm_size;
  double fbm_H;
  double *fbm_field; /* last generated fBm */
} AppState;

/** Print banner. */
static void banner(void) {
  printf("Superforce UI v%s\n", COINSORTER_VERSION_STR);
}

/** Display concise current state summary. */
static void show_state(const AppState *S) {
  const char *opt_name = "count";
  switch (S->opt_mode) {
    case OPT_MASS: opt_name = "mass"; break;
    case OPT_DIAMETER: opt_name = "diam"; break;
    case OPT_AREA: opt_name = "area"; break;
    default: opt_name = "count"; break;
  }
  printf("System=%s amt=%d opt=%s env=%s g=%.3f thermal=%s fbm=%d H=%.2f %s\n",
         S->coin_sys->system_name, S->amount, opt_name,
         S->env->name, S->env->g, S->thermal ? "on" : "off", S->fbm_size,
         S->fbm_H, S->fbm_field ? "[fbm]" : "");
}

/** Show command help. */
static void help_menu(void) {
  puts("Commands:\n"
       "  c/y cycle coin system (usd/eur)\n"
       "  a  set amount\n"
       "  o  cycle optimize mode (count/mass/diam/area)\n"
       "  e  cycle environment\n"
       "  t  toggle thermal Casimir\n"
       "  s  solve coin change\n"
       "  p  show physics sample\n"
       "  f  new fBm: prompt size,H then write ui_fbm.ppm\n"
       "  r  regenerate fBm with last params\n"
       "  P  Poisson solve on last fBm -> ui_poisson_phi.ppm\n"
       "  V  vector overlay -> ui_fbm_vectors.ppm\n"
       "  m  train tiny MLP demo (identity 2->2)\n"
       "  M  train MLP with debug loss prints\n"
       "  C  toggle color on/off\n"
       "  b  benchmark coin change\n"
       "  h  help\n"
       "  q  quit");
}

/** Cycle to next environment. */
static const Environment *cycle_env(const Environment *cur) {
  const char *order[] = {"earth", "moon", "mars", "orbit"};
  size_t n = sizeof(order) / sizeof(order[0]);
  size_t idx = 0;
  for (size_t i = 0; i < n; ++i)
    if (strcmp(cur->name, order[i]) == 0) {
      idx = i;
      break;
    }
  idx = (idx + 1) % n;
  return get_environment(order[idx]);
}

/** Compute and print coin change result. */
static void do_coin_change(const AppState *S) {
  int counts[32];
  memset(counts, 0, sizeof(counts));
  int use_greedy = (S->opt_mode == OPT_COUNT && S->coin_sys->canonical_hint);
  int status;
  if (S->opt_mode == OPT_COUNT) {
    if (use_greedy)
      status = greedy_make_change(S->coin_sys, S->amount, counts);
    else
      status = dp_make_change(S->coin_sys, S->amount, counts);
  } else {
    status = dp_make_change_opt(S->coin_sys, S->amount, counts, S->opt_mode);
  }
  if (status != 0) {
    printf("Change impossible for %d\n", S->amount);
    return;
  }
  double mass = total_mass(S->coin_sys, counts);
  double diam = total_diameter(S->coin_sys, counts);
  int total = 0;
  for (size_t i = 0; i < S->coin_sys->ncoins; ++i)
    total += counts[i];
  printf("Coins=%d mass=%.3fg diam=%.2fmm\n", total, mass, diam);
  for (size_t i = 0; i < S->coin_sys->ncoins; ++i)
    if (counts[i])
      printf("  %s x %d\n", S->coin_sys->coins[i].code, counts[i]);
}

/** Print sample physics values. */
static void do_physics(const AppState *S) {
  (void)S;
  printf("beta1=%.3e beta2=%.3e gamma_phi(1)=%.3e\n", beta1(), beta2(),
         gamma_phi(1.0));
  double R = 5e-6, d = 10e-9, T = 4.0, ani = 5.0;
  double F0 = casimir_base(R, d);
  double Fth = S->thermal ? casimir_thermal(R, d, T) : 0.0;
  for (int deg = 0; deg <= 180; deg += 60) {
    double th = deg * M_PI / 180.0;
    double Fm = casimir_modulated(F0, Fth, ani, th);
    printf("Casimir %3d %.4e\n", deg, Fm);
  }
}

/** Generate FBM field and write PPM(s). */
static void generate_fbm_field(AppState *S, int N, double H) {
  if (N < 3) {
    puts("size>=3");
    return;
  }
  if (S->fbm_field) {
    free(S->fbm_field);
    S->fbm_field = NULL;
  }
  S->fbm_field = (double *)malloc(sizeof(double) * N * N);
  if (!S->fbm_field) {
    puts("alloc fail");
    return;
  }
  S->fbm_size = N;
  S->fbm_H = H;
  if (fbm_diamond_square(S->fbm_field, N, H, 0) == 0) {
    write_field_ppm("ui_fbm.ppm", S->fbm_field, N, N);
    puts("wrote ui_fbm.ppm");
  } else {
    generate_fbm(S->fbm_field, N, N, H);
    write_field_ppm("ui_fbm_noise.ppm", S->fbm_field, N, N);
    puts("wrote ui_fbm_noise.ppm");
  }
}

/** Regenerate last fBm. */
static void regen_fbm(AppState *S) {
  if (!S->fbm_field) {
    puts("no fbm yet");
    return;
  }
  generate_fbm_field(S, S->fbm_size, S->fbm_H);
}

/** Solve Poisson on field and write solution. */
static void do_poisson(const AppState *S) {
  if (!S->fbm_field) {
    puts("no fbm yet");
    return;
  }
  int N = S->fbm_size;
  int NN = N * N;
  double *rhs = (double *)calloc(NN, sizeof(double));
  if (!rhs) {
    puts("alloc fail");
    return;
  }
  for (int y = 1; y < N - 1; ++y)
    for (int x = 1; x < N - 1; ++x) {
      int i = y * N + x;
      rhs[i] = 4 * S->fbm_field[i] - S->fbm_field[i - 1] - S->fbm_field[i + 1] -
               S->fbm_field[i - N] - S->fbm_field[i + N];
    }
  double *phi = (double *)calloc(NN, sizeof(double));
  if (!phi) {
    free(rhs);
    puts("alloc fail");
    return;
  }
  double res = poisson_jacobi(phi, rhs, N, N, 200);
  printf("poisson residual %.3e\n", res);
  write_field_ppm("ui_poisson_phi.ppm", phi, N, N);
  puts("wrote ui_poisson_phi.ppm");
  free(rhs);
  free(phi);
}

/** Compute vector field and overlay PPM. */
static void do_vectors(const AppState *S) {
  if (!S->fbm_field) {
    puts("no fbm yet");
    return;
  }
  int N = S->fbm_size;
  int NN = N * N;
  double *dx = (double *)malloc(sizeof(double) * NN);
  double *dy = (double *)malloc(sizeof(double) * NN);
  if (!dx || !dy) {
    free(dx);
    free(dy);
    puts("alloc fail");
    return;
  }
  compute_deflection(S->fbm_field, N, N, dx, dy);
  write_field_with_vectors_ppm("ui_fbm_vectors.ppm", S->fbm_field, dx, dy, N, N,
                               8);
  puts("wrote ui_fbm_vectors.ppm");
  free(dx);
  free(dy);
}

/** Benchmark DP/opt solver. */
static void do_bench(const AppState *S) {
  char in[64];
  printf("amount: ");
  if (!fgets(in, sizeof(in), stdin))
    return;
  int amt = atoi(in);
  printf("iters: ");
  if (!fgets(in, sizeof(in), stdin))
    return;
  int iters = atoi(in);
  if (amt <= 0 || iters <= 0) {
    puts("bad inputs");
    return;
  }
  int *tmp = (int *)calloc(S->coin_sys->ncoins, sizeof(int));
  if (!tmp) {
    puts("alloc fail");
    return;
  }
  clock_t t0, t1;
  double tot = 0, best = 1e9;
  for (int i = 0; i < iters; ++i) {
    memset(tmp, 0, S->coin_sys->ncoins * sizeof(int));
    t0 = clock();
    if (S->opt_mode == OPT_COUNT)
      dp_make_change(S->coin_sys, amt, tmp);
    else
      dp_make_change_opt(S->coin_sys, amt, tmp, S->opt_mode);
    t1 = clock();
    double dt = (double)(t1 - t0) / CLOCKS_PER_SEC;
    tot += dt;
    if (dt < best)
      best = dt;
  }
  printf("bench avg=%.4g best=%.4g sec\n", tot / iters, best);
  free(tmp);
}

/** Entry point for stand-alone UI. */
int main(void) {
  setlocale(LC_ALL, "");
  color_init();
  AppState S;
  memset(&S, 0, sizeof(S));
  S.coin_sys = get_coin_system("usd");
  S.amount = 137;
  S.opt_mode = OPT_COUNT;
  S.env = get_environment("earth");
  S.thermal = 1;
  S.fbm_size = 129;
  S.fbm_H = 0.5;
  S.fbm_field = NULL;
  banner();
  help_menu();
  show_state(&S);
  char line[128];
  while (1) {
    printf("%scmd>%s ", C_BOLD, C_RESET);
    if (!fgets(line, sizeof(line), stdin))
      break;
    char *p = line;
    while (*p && isspace((unsigned char)*p))
      ++p;
    char cmd = *p;
    if (cmd == '\0')
      continue;
    if (cmd == 'q')
      break;
    switch (cmd) {
    case 'h':
      help_menu();
      break;
    case 'c':
    case 'y':
      S.coin_sys = (strcmp(S.coin_sys->system_name, "usd") == 0)
                       ? get_coin_system("eur")
                       : get_coin_system("usd");
      break;
    case 'a':
      printf("new amount: ");
      if (fgets(line, sizeof(line), stdin)) {
        int v = atoi(line);
        if (v >= 0)
          S.amount = v;
      }
      break;
    case 'o':
      S.opt_mode = (S.opt_mode + 1) % OPT_MODE_COUNT;
      break;
    case 'e':
      S.env = cycle_env(S.env);
      break;
    case 't':
      S.thermal = !S.thermal;
      break;
    case 's':
      do_coin_change(&S);
      break;
    case 'p':
      do_physics(&S);
      break;
    case 'f':
      printf("size H: ");
      if (fgets(line, sizeof(line), stdin)) {
        int n;
        double H;
        if (sscanf(line, "%d %lf", &n, &H) == 2)
          generate_fbm_field(&S, n, H);
      }
      break;
    case 'r':
      regen_fbm(&S);
      break;
    case 'P':
      do_poisson(&S);
      break;
    case 'V':
      do_vectors(&S);
      break;
    case 'b':
      do_bench(&S);
      break;
    case 'm':
    case 'M': {
      int debug = (cmd == 'M');
      MLP mlp;
      if (mlp_init(&mlp, 2, 8, 2, 123) != 0) {
        puts("mlp init fail");
        break;
      }
      int n = 20;
      double xs[40];
      double ys[40];
      for (int i = 0; i < n; i++) {
        double a = (i / (double)(n - 1)) * 2 - 1;
        double b = 1 - a;
        xs[2 * i] = a;
        xs[2 * i + 1] = b;
        ys[2 * i] = a;
        ys[2 * i + 1] = b;
      }
      for (int e = 0; e < 300; e++) {
        mlp_train_epoch(&mlp, xs, ys, n, 0.01);
        if (debug && (e % 50 == 0)) {
          double loss = 0;
          double o[2];
          for (int i = 0; i < n; i++) {
            mlp_forward(&mlp, &xs[2 * i], o);
            double dx = o[0] - ys[2 * i];
            double dy = o[1] - ys[2 * i + 1];
            loss += dx * dx + dy * dy;
          }
          loss /= n;
          printf("epoch %d loss %.4g\n", e, loss);
        }
      }
      double test[2] = {0.3, -0.3};
      double out[2];
      mlp_forward(&mlp, test, out);
      printf("MLP test in(0.3,-0.3)->(%.3f,%.3f)\n", out[0], out[1]);
      mlp_free(&mlp);
    } break;
    case 'C':
      color_enabled = !color_enabled;
      puts(color_enabled ? "color on" : "color off");
      break;
    default:
      puts("unknown (h for help)");
      break;
    }
    show_state(&S);
  }
  free(S.fbm_field);
  return 0;
}
