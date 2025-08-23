/** \file beta.c
 *  \brief Simple perturbative beta / gamma functions (demonstration values).
 */
#include "beta.h"
#include "physics_constants.h"
#include <math.h>

double beta1(void) { return 3.0 / (16.0 * PHYSICS_PI_SQUARED); } /**< 1-loop beta coeff */
double beta2(void) {
  return -17.0 / (1536.0 * PHYSICS_PI_FOURTH);
}                                                   /**< 2-loop beta coeff */
double gamma_phi(double g) { return g * g / 12.0; } /**< Field anomalous dim */
