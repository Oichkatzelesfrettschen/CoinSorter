/** \file casimir.c
 *  \brief Simplified Casimir effect model functions.
 */
#include "casimir.h"
#include "physics_constants.h"
#include <math.h>

double casimir_base(double R, double d) {
  return pow(PHYSICS_PI, 3) * PHYSICS_HBAR * PHYSICS_C * R / (360.0 * pow(d, 3));
}

double casimir_thermal(double R, double d, double T) {
  double A_eff = 2.0 * PHYSICS_PI * R * d;
  return (pow(PHYSICS_PI, 3) * A_eff * pow(PHYSICS_KB * T, 4)) /
         (45.0 * pow(PHYSICS_HBAR, 3) * PHYSICS_C * PHYSICS_C * d * d);
}

double casimir_modulated(double F0, double Fth, double anisotropy,
                         double theta) {
  double mod = 1.0 + 0.5 * anisotropy * cos(theta);
  return (F0 + Fth) * mod;
}
