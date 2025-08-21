#include "simulation.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _MSC_VER
#define inline __inline
#endif
/* Portable fast RNG (xorshift32) to avoid rand() quality issues */
static unsigned rng_state = 2463534242u;
static inline unsigned xorshift32(void) {
  unsigned x = rng_state;
  x ^= x << 13;
  x ^= x >> 17;
  x ^= x << 5;
  rng_state = x;
  return x;
}
static inline double frand(void) {
  return (xorshift32() & 0xFFFFFF) / (double)0x1000000;
}

/**
 * @brief Generate fallback noise field (placeholder fBm) in [-0.5,0.5].
 */
void generate_fbm(double *field, int nx, int ny, double hurst) {
  (void)hurst;
  rng_state ^= (unsigned)time(NULL);
  for (int i = 0; i < nx * ny; ++i)
    field[i] = frand() - 0.5;
}
/* diamond-square requires square size 2^k + 1 */
/**
 * @brief Diamond-square fractal generator (fBm style) normalized to [-1,1].
 * @param f output buffer size N*N
 * @param N size (must be 2^k + 1)
 * @param H Hurst exponent (0,1)
 * @param seed RNG seed
 * @return 0 on success, -1 if size invalid
 */
int fbm_diamond_square(double *f, int N, double H, unsigned seed) {
  int size = N;
  int m = size - 1;
  if ((m & (m - 1)) != 0)
    return -1; /* not power of two plus 1 */
  rng_state ^= (seed ? seed : (unsigned)time(NULL));
  f[0] = frand() - 0.5;
  f[m] = frand() - 0.5;
  f[m * size] = frand() - 0.5;
  f[m * size + m] = frand() - 0.5;
  int step = m;
  double scale = 0.5;
  while (step > 1) {
    int half = step / 2;
    /* diamond */
    for (int y = half; y < m; y += step) {
      for (int x = half; x < m; x += step) {
        double a = f[(y - half) * size + (x - half)];
        double b = f[(y - half) * size + (x + half)];
        double c = f[(y + half) * size + (x - half)];
        double d = f[(y + half) * size + (x + half)];
        double avg = 0.25 * (a + b + c + d);
        double r = (frand() - 0.5) * 2.0 * scale;
        f[y * size + x] = avg + r;
      }
    }
    /* square */
    for (int y = 0; y <= m; y += half) {
      int shift = ((y / half) & 1) ? 0 : half;
      for (int x = shift; x <= m; x += step) {
        double sum = 0;
        int cnt = 0;
        if (x >= half) {
          sum += f[y * size + (x - half)];
          cnt++;
        }
        if (x + half <= m) {
          sum += f[y * size + (x + half)];
          cnt++;
        }
        if (y >= half) {
          sum += f[(y - half) * size + x];
          cnt++;
        }
        if (y + half <= m) {
          sum += f[(y + half) * size + x];
          cnt++;
        }
        double avg = cnt ? sum / cnt : 0.0;
        double r = (frand() - 0.5) * 2.0 * scale;
        f[y * size + x] = avg + r;
      }
    }
    step /= 2;
    scale *= pow(0.5, H);
  }
  /* normalize to [-1,1] */
  double mn = 1e9, mx = -1e9;
  for (int i = 0; i < size * size; ++i) {
    if (f[i] < mn)
      mn = f[i];
    if (f[i] > mx)
      mx = f[i];
  }
  if (mx > mn) {
    double inv = 2.0 / (mx - mn);
    for (int i = 0; i < size * size; ++i)
      f[i] = -1.0 + (f[i] - mn) * inv;
  }
  return 0;
}

/**
 * @brief Write field as grayscale PPM (auto normalize).
 */
int write_field_ppm(const char *filename, const double *field, int nx, int ny) {
  FILE *fp = fopen(filename, "wb");
  if (!fp)
    return 0;
  fprintf(fp, "P6\n%d %d\n255\n", nx, ny);
  double mn = 1e9, mx = -1e9;
  int N = nx * ny;
  for (int i = 0; i < N; ++i) {
    if (field[i] < mn)
      mn = field[i];
    if (field[i] > mx)
      mx = field[i];
  }
  double inv = (mx > mn) ? 1.0 / (mx - mn) : 1.0;
  for (int i = 0; i < N; ++i) {
    double v = (field[i] - mn) * inv;
    unsigned char g = (unsigned char)(v * 255.0 + 0.5);
    unsigned char rgb[3] = {g, g, g};
    fwrite(rgb, 1, 3, fp);
  }
  fclose(fp);
  return 1;
}
void forward_raytrace(const double *field, int nx, int ny) {
  (void)field;
  (void)nx;
  (void)ny;
  puts("[sim] forward_raytrace stub");
}
void inverse_retrieve(const double *obs, int n, double *recon) {
  (void)obs;
  (void)n;
  (void)recon;
  puts("[sim] inverse_retrieve stub");
}

double poisson_jacobi(double *phi, const double *rhs, int nx, int ny,
                      int iters) {
  double *next = (double *)malloc(sizeof(double) * nx * ny);
  if (!next)
    return -1;
  memcpy(next, phi, sizeof(double) * nx * ny);
  double res = 0;
  for (int it = 0; it < iters; ++it) {
    res = 0;
    for (int y = 1; y < ny - 1; ++y) {
      for (int x = 1; x < nx - 1; ++x) {
        int i = y * nx + x;
        double newv = 0.25 * (phi[i - 1] + phi[i + 1] + phi[i - nx] +
                              phi[i + nx] - rhs[i]);
        res += fabs(newv - phi[i]);
        next[i] = newv;
      }
    }
    memcpy(phi + nx, next + nx,
           sizeof(double) * (nx * (ny - 2))); /* keep boundary */
    res /= (double)((nx - 2) * (ny - 2));
  }
  free(next);
  return res;
}

void compute_deflection(const double *field, int nx, int ny, double *out_dx,
                        double *out_dy) {
  for (int y = 0; y < ny; ++y) {
    for (int x = 0; x < nx; ++x) {
      int i = y * nx + x;
      double fx_l = field[y * nx + (x > 0 ? x - 1 : x)];
      double fx_r = field[y * nx + (x < nx - 1 ? x + 1 : x)];
      double fy_u = field[(y > 0 ? y - 1 : y) * nx + x];
      double fy_d = field[(y < ny - 1 ? y + 1 : y) * nx + x];
      out_dx[i] = 0.5 * (fx_r - fx_l);
      out_dy[i] = 0.5 * (fy_d - fy_u);
    }
  }
}

int write_field_with_vectors_ppm(const char *filename, const double *field,
                                 const double *dx, const double *dy, int nx,
                                 int ny, int stride) {
  FILE *fp = fopen(filename, "wb");
  if (!fp)
    return 0;
  fprintf(fp, "P6\n%d %d\n255\n", nx, ny);
  double mn = 1e9, mx = -1e9;
  int N = nx * ny;
  for (int i = 0; i < N; ++i) {
    if (field[i] < mn)
      mn = field[i];
    if (field[i] > mx)
      mx = field[i];
  }
  double inv = (mx > mn) ? 1.0 / (mx - mn) : 1.0;
  for (int y = 0; y < ny; ++y) {
    for (int x = 0; x < nx; ++x) {
      int i = y * nx + x;
      double v = (field[i] - mn) * inv;
      unsigned char r, g, b;
      r = g = b = (unsigned char)(v * 255.0 + 0.5);
      if (dx && dy && (x % stride == 0) && (y % stride == 0)) {
        double vx = dx[i];
        double vy = dy[i];
        double mag = sqrt(vx * vx + vy * vy);
        if (mag > 0) {
          double s = fmin(mag * 10.0, 1.0);
          r = (unsigned char)(255 * s);
          g = (unsigned char)(128 * (1.0 - s));
          b = 0;
        }
      }
      unsigned char rgb[3] = {r, g, b};
      fwrite(rgb, 1, 3, fp);
    }
  }
  fclose(fp);
  return 1;
}

int mlp_init(MLP *m, int in_dim, int hid_dim, int out_dim, unsigned seed) {
  m->in_dim = in_dim;
  m->hid_dim = hid_dim;
  m->out_dim = out_dim;
  size_t n1 = (size_t)in_dim * hid_dim;
  size_t n2 = (size_t)hid_dim * out_dim;
  m->w1 = (double *)malloc(sizeof(double) * n1);
  m->b1 = (double *)calloc(hid_dim, sizeof(double));
  m->w2 = (double *)malloc(sizeof(double) * n2);
  m->b2 = (double *)calloc(out_dim, sizeof(double));
  if (!m->w1 || !m->b1 || !m->w2 || !m->b2)
    return -1;
  srand(seed);
  for (size_t i = 0; i < n1; ++i)
    m->w1[i] = (frand() - 0.5) * 0.2;
  for (size_t i = 0; i < n2; ++i)
    m->w2[i] = (frand() - 0.5) * 0.2;
  return 0;
}
void mlp_free(MLP *m) {
  if (!m)
    return;
  free(m->w1);
  free(m->b1);
  free(m->w2);
  free(m->b2);
  memset(m, 0, sizeof(*m));
}
void mlp_forward(const MLP *m, const double *x, double *y) {
  double *h = (double *)malloc(sizeof(double) * m->hid_dim);
  for (int j = 0; j < m->hid_dim; ++j) {
    double acc = m->b1[j];
    for (int i = 0; i < m->in_dim; ++i)
      acc += x[i] * m->w1[i * m->hid_dim + j];
    h[j] = acc > 0 ? acc : 0;
  }
  for (int k = 0; k < m->out_dim; ++k) {
    double acc = m->b2[k];
    for (int j = 0; j < m->hid_dim; ++j)
      acc += h[j] * m->w2[j * m->out_dim + k];
    y[k] = acc;
  }
  free(h);
}
void mlp_train_epoch(MLP *m, const double *xs, const double *ys, int n_samples,
                     double lr) {
  /* Simple SGD with MSE, no batching */
  for (int n = 0; n < n_samples; ++n) {
    const double *x = &xs[n * m->in_dim];
    const double *t = &ys[n * m->out_dim];
    double *h = (double *)malloc(sizeof(double) * m->hid_dim);
    unsigned char *mask = (unsigned char *)malloc(m->hid_dim);
    for (int j = 0; j < m->hid_dim; ++j) {
      double acc = m->b1[j];
      for (int i = 0; i < m->in_dim; ++i)
        acc += x[i] * m->w1[i * m->hid_dim + j];
      if (acc > 0) {
        h[j] = acc;
        mask[j] = 1;
      } else {
        h[j] = 0;
        mask[j] = 0;
      }
    }
    double *o = (double *)malloc(sizeof(double) * m->out_dim);
    for (int k = 0; k < m->out_dim; ++k) {
      double acc = m->b2[k];
      for (int j = 0; j < m->hid_dim; ++j)
        acc += h[j] * m->w2[j * m->out_dim + k];
      o[k] = acc;
    }
    double *grad_o = (double *)malloc(sizeof(double) * m->out_dim);
    for (int k = 0; k < m->out_dim; ++k)
      grad_o[k] = (o[k] - t[k]);
    /* w2,b2 */
    for (int k = 0; k < m->out_dim; ++k) {
      m->b2[k] -= lr * grad_o[k];
      for (int j = 0; j < m->hid_dim; ++j)
        m->w2[j * m->out_dim + k] -= lr * grad_o[k] * h[j];
    }
    /* backprop into h */
    double *grad_h = (double *)calloc(m->hid_dim, sizeof(double));
    for (int j = 0; j < m->hid_dim; ++j) {
      double sum = 0;
      for (int k = 0; k < m->out_dim; ++k)
        sum += grad_o[k] * m->w2[j * m->out_dim + k];
      grad_h[j] = mask[j] ? sum : 0;
    }
    /* w1,b1 */
    for (int j = 0; j < m->hid_dim; ++j) {
      m->b1[j] -= lr * grad_h[j];
      for (int i = 0; i < m->in_dim; ++i)
        m->w1[i * m->hid_dim + j] -= lr * grad_h[j] * x[i];
    }
    free(h);
    free(mask);
    free(o);
    free(grad_o);
    free(grad_h);
  }
}
