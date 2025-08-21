#include "beta.h"
#include "casimir.h"
#include "coins.h"
#include "color.h"
#include "env.h"
#include "simulation.h"
#include "version.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void physics_block(void) {
  printf("=== phi^4 RG ===\n");
  printf("beta1=%.12e\n", beta1());
  printf("beta2=%.12e\n", beta2());
  printf("gamma_phi(g=1)=%.12e\n\n", gamma_phi(1.0));
  printf("=== Casimir Sweep ===\n");
  double R = 5e-6, d = 10e-9, T = 4.0, ani = 5.0;
  double F0 = casimir_base(R, d);
  double Fth = casimir_thermal(R, d, T);
  for (int deg = 0; deg <= 180; deg += 30) {
    double th = deg * M_PI / 180.0;
    double Fm = casimir_modulated(F0, Fth, ani, th);
    printf("%3d %.6e\n", deg, Fm);
  }
  puts("");
}

static void simulation_block(void) {
  int nx = 64, ny = 64;
  double *f = (double *)malloc(sizeof(double) * nx * ny);
  generate_fbm(f, nx, ny, 0.7);
  forward_raytrace(f, nx, ny);
  inverse_retrieve(f, nx * ny, f);
  free(f);
}

int main(int argc, char **argv) {
  color_init();
  int do_phys = 0, do_sim = 0;
  int fbm_size = 129; /* 2^7+1 default */
  double fbm_H = 0.5;
  int save_fbm = 0;
  int do_poisson = 0;
  int do_vectors = 0;
  const char *system = "usd";
  int amount = 137;
  int json = 0;
  int show_version = 0;
  OptimizeMode opt_mode = OPT_COUNT;
  int no_thermal = 0;
  const char *env_name = "earth";
  int force_no_color = 0;
  for (int i = 1; i < argc; ++i) {
    if (!strcmp(argv[i], "--physics"))
      do_phys = 1;
    else if (!strcmp(argv[i], "--sim"))
      do_sim = 1;
    else if (!strncmp(argv[i], "fbmSize=", 8))
      fbm_size = atoi(argv[i] + 8);
    else if (!strncmp(argv[i], "fbmH=", 5))
      fbm_H = strtod(argv[i] + 5, NULL);
    else if (!strcmp(argv[i], "--fbm-ppm"))
      save_fbm = 1;
    else if (!strcmp(argv[i], "--poisson"))
      do_poisson = 1;
    else if (!strcmp(argv[i], "--vectors"))
      do_vectors = 1;
    else if (!strcmp(argv[i], "--json"))
      json = 1;
    else if (!strcmp(argv[i], "--version"))
      show_version = 1;
    else if (!strcmp(argv[i], "--no-thermal"))
      no_thermal = 1;
    else if (!strcmp(argv[i], "--no-color"))
      force_no_color = 1;
    else if (!strncmp(argv[i], "opt=", 4)) {
      const char *m = argv[i] + 4;
      if (!strcmp(m, "count"))
        opt_mode = OPT_COUNT;
      else if (!strcmp(m, "mass"))
        opt_mode = OPT_MASS;
      else if (!strcmp(m, "diam"))
        opt_mode = OPT_DIAMETER;
      else if (!strcmp(m, "area"))
        opt_mode = OPT_AREA;
    }
    if (show_version) {
      printf("superforce version %s\n", COINSORTER_VERSION_STR);
    } else if (!strncmp(argv[i], "sys=", 4))
      system = argv[i] + 4;
    else if (!strncmp(argv[i], "amt=", 4))
      amount = atoi(argv[i] + 4);
    else if (!strncmp(argv[i], "env=", 4))
      env_name = argv[i] + 4;
  }
  const Environment *env = get_environment(env_name);
  if (!env) {
    char msg[128];
    snprintf(msg, sizeof(msg), "Unknown env %s\n", env_name);
    fputs(msg, stderr);
    return 1;
  }
  if (do_phys) {
    printf("%s", C_BOLD);
    physics_block();
    printf("%sEnvironment:%s %s g=%.3f m/s^2  T=%.1fK  P=%.3fkPa\n", C_CYAN,
           C_RESET, env->name, env->g, env->temperature_K, env->pressure_kPa);
    if (no_thermal)
      printf("%s[casimir]%s thermal contribution disabled\n", C_YELLOW,
             C_RESET);
    if (force_no_color)
      color_enabled = 0;
  }
  if (do_sim) {
    /* If square fBm request */
    if (fbm_size > 3) {
      double *fbm = (double *)malloc(sizeof(double) * fbm_size * fbm_size);
      if (fbm_diamond_square(fbm, fbm_size, fbm_H, 0) == 0) {
        if (save_fbm)
          write_field_ppm("fbm.ppm", fbm, fbm_size, fbm_size);
      } else {
        /* fallback noise */
        generate_fbm(fbm, fbm_size, fbm_size, fbm_H);
        if (save_fbm)
          write_field_ppm("fbm_noise.ppm", fbm, fbm_size, fbm_size);
      }
      if (do_poisson) {
        double *rhs = (double *)calloc(fbm_size * fbm_size, sizeof(double));
        /* simple rhs: laplacian of fbm approximation */
        for (int y = 1; y < fbm_size - 1; ++y) {
          for (int x = 1; x < fbm_size - 1; ++x) {
            int i = y * fbm_size + x;
            rhs[i] = 4 * fbm[i] - fbm[i - 1] - fbm[i + 1] - fbm[i - fbm_size] -
                     fbm[i + fbm_size];
          }
        }
        double *phi = (double *)calloc(fbm_size * fbm_size, sizeof(double));
        double res = poisson_jacobi(phi, rhs, fbm_size, fbm_size, 200);
        char pmsg[128];
        snprintf(pmsg, sizeof(pmsg), "[poisson] residual=%.3e\n", res);
        fputs(pmsg, stderr);
        write_field_ppm("poisson_phi.ppm", phi, fbm_size, fbm_size);
        free(rhs);
        free(phi);
      }
      if (do_vectors) {
        double *dx = (double *)malloc(sizeof(double) * fbm_size * fbm_size);
        double *dy = (double *)malloc(sizeof(double) * fbm_size * fbm_size);
        compute_deflection(fbm, fbm_size, fbm_size, dx, dy);
        write_field_with_vectors_ppm("fbm_vectors.ppm", fbm, dx, dy, fbm_size,
                                     fbm_size, 8);
        free(dx);
        free(dy);
      }
      free(fbm);
    }
    simulation_block();
  }
  const CoinSystem *cs = get_coin_system(system);
  if (!cs) {
    char msg[128];
    snprintf(msg, sizeof(msg), "Unknown system %s\n", system);
    fputs(msg, stderr);
    return 1;
  }
  int *counts = (int *)calloc(cs->ncoins, sizeof(int));
  int ex = -1;
  int use_greedy = cs->canonical_hint;
  if (use_greedy && !audit_canonical(cs, amount > 500 ? 500 : amount, &ex))
    use_greedy = 0;
  if (opt_mode != OPT_COUNT) {
    use_greedy = 0;
    dp_make_change_opt(cs, amount, counts, opt_mode);
  } else if (use_greedy)
    greedy_make_change(cs, amount, counts);
  else
    dp_make_change(cs, amount, counts);
  if (json) {
    char buf[768];
    const char *strategy =
        (opt_mode == OPT_MASS
             ? "dp-mass"
             : (opt_mode == OPT_DIAMETER
                    ? "dp-diam"
                    : (opt_mode == OPT_AREA ? "dp-area"
                                            : (use_greedy ? "greedy" : "dp"))));
    if (format_change_json(cs, amount, counts, strategy, COINSORTER_VERSION_STR,
                           buf, sizeof(buf)) == 0)
      puts(buf);
  } else {
    printf("%sSystem=%s%s amount=%d strategy=%s%s%s\n", C_BOLD, cs->system_name,
           C_RESET, amount, use_greedy ? C_GREEN : C_MAGENTA,
           use_greedy ? "greedy" : "dp", C_RESET);
    double mass = total_mass(cs, counts);
    for (size_t i = 0; i < cs->ncoins; ++i)
      if (counts[i])
        printf("  %s %d x %d\n", cs->coins[i].name, cs->coins[i].value,
               counts[i]);
    if (mass > 0)
      printf("Total mass: %s%.3f g%s\n", C_BLUE, mass, C_RESET);
    if (!use_greedy)
      printf("%s(greedy first fails at %d)%s\n", C_RED, ex, C_RESET);
    printf("Env=%s%s%s g=%.3f m/s^2\n", C_CYAN, env->name, C_RESET, env->g);
  }
  free(counts);
  return 0;
}
