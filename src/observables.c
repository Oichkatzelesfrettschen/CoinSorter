/** \file observables.c
 *  \brief Additional physics observables (placeholder for future expansion).
 *
 *  This stub demonstrates where extended measurable quantities (e.g.,
 *  derived energy densities, scaling functions, or statistical aggregations
 *  over simulation fields) can be implemented and linked into the core
 *  library for UI overlays.
 */
#include <math.h>

/** \brief Normalized energy-like scalar from gradient components.
 *
 * Current implementation returns 0.5*(dx^2+dy^2). In future this could be
 * extended to incorporate dielectric or material parameters.
 */
double observable_energy_density(double dx, double dy) {
  return 0.5 * (dx * dx + dy * dy);
}
