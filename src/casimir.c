#include "casimir.h"
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
static const double HBAR = 1.054571817e-34;
static const double C0 = 2.99792458e8;
static const double KB = 1.380649e-23;
static const double PI = M_PI;

double casimir_base(double R, double d) {
  return pow(PI, 3) * HBAR * C0 * R / (360.0 * pow(d, 3));
}

double casimir_thermal(double R, double d, double T) {
  double A_eff = 2.0 * PI * R * d;
  return (pow(PI, 3) * A_eff * pow(KB * T, 4)) /
         (45.0 * pow(HBAR, 3) * C0 * C0 * d * d);
}

double casimir_modulated(double F0, double Fth, double anisotropy,
                         double theta) {
  double mod = 1.0 + 0.5 * anisotropy * cos(theta);
  return (F0 + Fth) * mod;
}
