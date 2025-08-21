#ifndef SIMULATION_H
#define SIMULATION_H
/** \file simulation.h
 *  \brief Procedural simulation helpers: fractal terrain, Poisson solve, vector
 * field, simple MLP.
 */

/** \brief Generate fallback fBm-like noise (simple) in range roughly
 * [-0.5,0.5]. */
void generate_fbm(double *field, int nx, int ny, double hurst);
/** \brief Generate alternative value-noise field (tileable) normalized to
 * [-1,1]. */
void generate_value_noise(double *field, int nx, int ny, unsigned seed,
                          int octaves);
/** \brief Stub forward raytrace (placeholder). */
void forward_raytrace(const double *field, int nx, int ny);
/** \brief Stub inverse retrieval (placeholder). */
void inverse_retrieve(const double *obs, int n, double *recon);

/** \brief Jacobi Poisson solver for Laplacian(phi)=rhs (Dirichlet boundary
 * retained). */
double poisson_jacobi(double *phi, const double *rhs, int nx, int ny,
                      int iters);

/** \brief Compute central-difference gradient (deflection) of scalar field. */
void compute_deflection(const double *field, int nx, int ny, double *out_dx,
                        double *out_dy);

/** \brief Write PPM with optional vector overlay (color-coded magnitude). */
int write_field_with_vectors_ppm(const char *filename, const double *field,
                                 const double *dx, const double *dy, int nx,
                                 int ny, int stride);

/** \brief Single hidden-layer MLP scaffold. */
typedef struct {
  int in_dim;  /**< Input dimension. */
  int hid_dim; /**< Hidden layer size. */
  int out_dim; /**< Output dimension. */
  double *w1;  /**< Weights input->hidden (in_dim*hid_dim). */
  double *b1;  /**< Bias hidden (hid_dim). */
  double *w2;  /**< Weights hidden->output (hid_dim*out_dim). */
  double *b2;  /**< Bias output (out_dim). */
} MLP;

/** \brief Initialize MLP parameter buffers with small random weights. */
int mlp_init(MLP *m, int in_dim, int hid_dim, int out_dim, unsigned seed);
/** \brief Release allocated buffers and zero struct. */
void mlp_free(MLP *m);
/** \brief Forward pass for single sample. */
void mlp_forward(const MLP *m, const double *x, double *y);
/** \brief One training epoch (SGD over all samples, MSE loss, ReLU
 * activations). */
void mlp_train_epoch(MLP *m, const double *xs, const double *ys, int n_samples,
                     double lr);

/** \brief Diamond-square fractal heightfield generator (N must be 2^k+1). */
int fbm_diamond_square(double *field, int N, double hurst, unsigned seed);

/** \brief Write grayscale height map (auto-normalized if needed) to PPM. */
int write_field_ppm(const char *filename, const double *field, int nx, int ny);

#endif /* SIMULATION_H */
