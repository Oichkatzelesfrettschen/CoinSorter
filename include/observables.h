/** \file observables.h
 *  \brief Declarations for additional physics / simulation observables.
 */
#ifndef OBSERVABLES_H
#define OBSERVABLES_H
#ifdef __cplusplus
extern "C" {
#endif

/** \brief Normalized energy-like density from gradient components.
 *
 * Computes 0.5*(dx^2+dy^2). This is analogous to a local energy density of a
 * scalar field whose gradient components are (dx,dy).
 * \param dx Local x-gradient (central difference or similar).
 * \param dy Local y-gradient.
 * \return Non-negative scalar proportional to local energy density.
 */
double observable_energy_density(double dx, double dy);

#ifdef __cplusplus
}
#endif
#endif /* OBSERVABLES_H */
