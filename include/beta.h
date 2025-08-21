/** \file beta.h
 *  \brief φ^4 theory MS-bar renormalization group coefficients.
 */
#ifndef BETA_H
#define BETA_H

/** First β-function coefficient β₁ = 3/(16π²). */
double beta1(void);
/** Second β-function coefficient β₂ = -17/(1536 π⁴). */
double beta2(void);
/** Two-loop anomalous dimension γ_φ(g) = g²/12. */
double gamma_phi(double g);

#endif /* BETA_H */
