/** \file beta.c
 *  \brief Simple perturbative beta / gamma functions (demonstration values).
 */
#include "beta.h"
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
static const double PI = M_PI;

double beta1(void) { return 3.0 / (16.0 * PI * PI); } /**< 1-loop beta coeff */
double beta2(void) {
  return -17.0 / (1536.0 * pow(PI, 4));
}                                                   /**< 2-loop beta coeff */
double gamma_phi(double g) { return g * g / 12.0; } /**< Field anomalous dim */
