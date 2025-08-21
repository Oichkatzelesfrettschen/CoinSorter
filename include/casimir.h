/** \file casimir.h
 *  \brief Casimir force models (sphere-plate PFA + modulation).
 */
#ifndef CASIMIR_H
#define CASIMIR_H

/** Base PFA sphere-plate force F = π^3 ħ c R / (360 d^3). */
double casimir_base(double R, double d);
/** Thermal correction (approximate). */
double casimir_thermal(double R, double d, double T);
/** Modulated force (F0+Fth)*(1+0.5*anisotropy*cos(theta)). */
double casimir_modulated(double F0, double Fth, double anisotropy,
                         double theta);

#endif /* CASIMIR_H */
