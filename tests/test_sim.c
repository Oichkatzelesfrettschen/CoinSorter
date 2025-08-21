#include "simulation.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
  int N = 33;
  int NN = N * N;
  double *f = malloc(sizeof(double) * NN);
  if (!f)
    return 1;
  generate_fbm(f, N, N, 0.6);
  double *rhs = calloc(NN, sizeof(double));
  if (!rhs)
    return 1;
  for (int y = 1; y < N - 1; ++y)
    for (int x = 1; x < N - 1; ++x) {
      int i = y * N + x;
      rhs[i] = 4 * f[i] - f[i - 1] - f[i + 1] - f[i - N] - f[i + N];
    }
  double *phi = calloc(NN, sizeof(double));
  if (!phi)
    return 1;
  double r1 = poisson_jacobi(phi, rhs, N, N, 10);
  double r2 = poisson_jacobi(phi, rhs, N, N, 40);
  if (!(r2 < r1)) {
    fprintf(stderr, "residual not decreasing %g -> %g\n", r1, r2);
    return 1;
  }
  MLP mlp;
  if (mlp_init(&mlp, 2, 6, 2, 42) != 0) {
    fprintf(stderr, "mlp init fail\n");
    return 1;
  }
  int samples = 20;
  double *xs = malloc(sizeof(double) * 2 * samples);
  double *ys = malloc(sizeof(double) * 2 * samples);
  for (int i = 0; i < samples; i++) {
    double a = (i / (double)(samples - 1)) * 2 - 1;
    double b = 1 - a;
    xs[2 * i] = a;
    xs[2 * i + 1] = b;
    ys[2 * i] = a;
    ys[2 * i + 1] = b;
  }
  for (int e = 0; e < 150; e++)
    mlp_train_epoch(&mlp, xs, ys, samples, 0.02);
  double t[2] = {0.25, 0.75};
  double o[2];
  mlp_forward(&mlp, t, o);
  if (fabs(o[0] - 0.25) > 0.2 || fabs(o[1] - 0.75) > 0.2) {
    fprintf(stderr, "mlp poor fit (%.3f,%.3f)\n", o[0], o[1]);
    return 1;
  }
  mlp_free(&mlp);
  free(f);
  free(rhs);
  free(phi);
  free(xs);
  free(ys);
  /* Color disable logic (indirect): just ensure color_init doesn't crash with
   * NO_COLOR set */
  setenv("NO_COLOR", "1", 1);
  extern void color_init(void);
  extern int color_enabled;
  color_enabled = 1;
  color_init();
  if (color_enabled != 0) {
    fprintf(stderr, "NO_COLOR not respected\n");
    return 1;
  }
  printf("sim tests passed\n");
  return 0;
}
