/** \file physics_constants.h
 *  \brief Centralized physical constants for all physics modules.
 *
 *  This header provides a unified source of physical constants to eliminate
 *  redundancy across physics modules and ensure consistency.
 */
#ifndef PHYSICS_CONSTANTS_H
#define PHYSICS_CONSTANTS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Mathematical constants */
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/** π (pi) constant */
#define PHYSICS_PI M_PI

/** 2π */
#define PHYSICS_TWO_PI (2.0 * PHYSICS_PI)

/** π² */
#define PHYSICS_PI_SQUARED (PHYSICS_PI * PHYSICS_PI)

/** π⁴ */
#define PHYSICS_PI_FOURTH (PHYSICS_PI_SQUARED * PHYSICS_PI_SQUARED)

/* Fundamental physical constants (SI units) */

/** Reduced Planck constant (ℏ) in J·s */
#define PHYSICS_HBAR 1.054571817e-34

/** Speed of light in vacuum (c) in m/s */
#define PHYSICS_C 2.99792458e8

/** Boltzmann constant (kB) in J/K */
#define PHYSICS_KB 1.380649e-23

/** Vacuum permittivity (ε₀) in F/m */
#define PHYSICS_EPSILON0 8.854187817e-12

/** Vacuum permeability (μ₀) in H/m */
#define PHYSICS_MU0 (4.0e-7 * PHYSICS_PI)

/** Elementary charge (e) in C */
#define PHYSICS_E 1.602176634e-19

/** Electron mass (mₑ) in kg */
#define PHYSICS_ME 9.1093837015e-31

/** Proton mass (mp) in kg */
#define PHYSICS_MP 1.67262192369e-27

/** Fine structure constant (α) dimensionless */
#define PHYSICS_ALPHA 7.2973525693e-3

/** Avogadro constant (NA) in mol⁻¹ */
#define PHYSICS_NA 6.02214076e23

/** Gas constant (R) in J/(mol·K) */
#define PHYSICS_R 8.314462618

/* Derived combinations commonly used in physics calculations */

/** ℏc in J·m */
#define PHYSICS_HBAR_C (PHYSICS_HBAR * PHYSICS_C)

/** ℏ²c in J²·m·s */
#define PHYSICS_HBAR2_C (PHYSICS_HBAR * PHYSICS_HBAR * PHYSICS_C)

/** kB/ℏc for thermal corrections in m⁻¹·K⁻¹ */
#define PHYSICS_KB_OVER_HBAR_C (PHYSICS_KB / PHYSICS_HBAR_C)

/** α/π for QED calculations */
#define PHYSICS_ALPHA_OVER_PI (PHYSICS_ALPHA / PHYSICS_PI)

#ifdef __cplusplus
}
#endif

#endif /* PHYSICS_CONSTANTS_H */