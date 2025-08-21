#ifndef SIMULATION_H
#define SIMULATION_H

void generate_fbm(double *field, int nx, int ny, double hurst);
void forward_raytrace(const double *field, int nx, int ny);
void inverse_retrieve(const double *obs, int n, double *recon);

/* Poisson solver (Jacobi iterations) for Laplacian(phi)=rhs on interior with
 * Dirichlet boundary (phi boundary fixed). Returns residual after iterations.
 */
double poisson_jacobi(double *phi, const double *rhs, int nx, int ny,
                      int iters);

/* Compute deflection (simple gradient) field from scalar map. out_dx/out_dy
 * length nx*ny. */
void compute_deflection(const double *field, int nx, int ny, double *out_dx,
                        double *out_dy);

/* Write an overlay PPM: base scalar plus optional vectors (dx,dy) subsampled.
 */
int write_field_with_vectors_ppm(const char *filename, const double *field,
                                 const double *dx, const double *dy, int nx,
                                 int ny, int stride);

/* Minimal MLP scaffold (single hidden layer) for experimentation */
typedef struct {
  int in_dim, hid_dim, out_dim;
  double *w1; /* in_dim * hid_dim */
  double *b1; /* hid_dim */
  double *w2; /* hid_dim * out_dim */
  double *b2; /* out_dim */
} MLP;

int mlp_init(MLP *m, int in_dim, int hid_dim, int out_dim, unsigned seed);
void mlp_free(MLP *m);
void mlp_forward(const MLP *m, const double *x, double *y); /* single sample */
void mlp_train_epoch(MLP *m, const double *xs, const double *ys, int n_samples,
                     double lr);

/* Higher quality fBm (diamond-square) for square grids size=2^n+1. Returns 0 on
 * success, -1 on fallback to noise. */
int fbm_diamond_square(double *field, int N, double hurst, unsigned seed);

/* Write grayscale PPM (P6) from field assumed in [-1,1] or arbitrary
 * (auto-normalized). */
int write_field_ppm(const char *filename, const double *field, int nx, int ny);

#endif /* SIMULATION_H */
