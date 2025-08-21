/** \file observables.h
 *  \brief Declarations for additional physics / simulation observables.
 */
#ifndef OBSERVABLES_H
#define OBSERVABLES_H

#include <stddef.h>
#include "coins.h"

#ifdef __cplusplus
extern "C" {
#endif

/** \brief Comprehensive material properties for physical calculations.
 *
 * Derived from exhaustive research of coin material properties including
 * electromagnetic, thermal, mechanical, and optical characteristics.
 */
typedef struct {
  /* Electromagnetic properties */
  double relative_permittivity;    /**< Relative permittivity (εr) dimensionless */
  double electrical_conductivity;  /**< Electrical conductivity (S/m) */
  double relative_permeability;    /**< Relative permeability (μr) dimensionless */
  double magnetic_susceptibility;  /**< Magnetic susceptibility (dimensionless) */
  
  /* Thermal properties */
  double thermal_conductivity;     /**< Thermal conductivity (W/(m·K)) */
  double specific_heat_capacity;   /**< Specific heat capacity (J/(kg·K)) */
  double thermal_expansion_coeff;  /**< Linear thermal expansion coefficient (1/K) */
  
  /* Mechanical properties */
  double density;                  /**< Density (kg/m³) */
  double youngs_modulus;          /**< Young's modulus (GPa) */
  double poissons_ratio;          /**< Poisson's ratio (dimensionless) */
  double hardness_hv;             /**< Vickers hardness (HV) */
  
  /* Optical properties */
  double refractive_index;        /**< Refractive index (dimensionless) */
  double reflectance;             /**< Reflectance coefficient (dimensionless) */
  
  /* Chemical properties */
  double corrosion_resistance;    /**< Corrosion resistance index (dimensionless) */
  
  const char *material_class;     /**< Material classification string */
} MaterialProperties;

/** \brief Normalized energy-like density from gradient components.
 *
 * Computes 0.5*(dx^2+dy^2). This is analogous to a local energy density of a
 * scalar field whose gradient components are (dx,dy).
 * \param dx Local x-gradient (central difference or similar).
 * \param dy Local y-gradient.
 * \return Non-negative scalar proportional to local energy density.
 */
double observable_energy_density(double dx, double dy);

/** \brief Enhanced energy density incorporating material properties.
 *
 * Computes energy density including dielectric and conductive effects.
 * \param dx Local x-gradient component.
 * \param dy Local y-gradient component.
 * \param properties Material properties of the coin.
 * \return Enhanced energy density incorporating material effects.
 */
double observable_energy_density_enhanced(double dx, double dy, 
                                         const MaterialProperties *properties);

/** \brief Electromagnetic field energy density for coin material.
 *
 * Calculates electromagnetic field energy density based on material properties.
 * \param e_field Electric field magnitude (V/m).
 * \param b_field Magnetic flux density magnitude (T).
 * \param properties Material properties.
 * \return EM field energy density (J/m³).
 */
double observable_em_energy_density(double e_field, double b_field,
                                   const MaterialProperties *properties);

/** \brief Thermal diffusion coefficient from material properties.
 *
 * Computes thermal diffusivity α = k/(ρ·cp).
 * \param properties Material properties.
 * \return Thermal diffusivity (m²/s).
 */
double observable_thermal_diffusivity(const MaterialProperties *properties);

/** \brief Skin depth for electromagnetic waves in conducting material.
 *
 * Calculates skin depth δ = √(2/(ωμσ)) for AC fields.
 * \param frequency Angular frequency (rad/s).
 * \param properties Material properties.
 * \return Skin depth (m).
 */
double observable_skin_depth(double frequency, const MaterialProperties *properties);

/** \brief Acoustic impedance of the material.
 *
 * Computes Z = ρ·v where v is calculated from elastic modulus.
 * \param properties Material properties.
 * \return Acoustic impedance (kg/(m²·s)).
 */
double observable_acoustic_impedance(const MaterialProperties *properties);

/** \brief Look up material properties from coin composition string.
 *
 * Analyzes composition string and returns appropriate material properties.
 * \param coin Coin specification containing composition.
 * \return Pointer to material properties, or NULL if not found.
 */
const MaterialProperties *get_material_properties(const CoinSpec *coin);

/** \brief Get material properties by composition string directly.
 *
 * Direct lookup by composition string for more flexible usage.
 * \param composition Material composition string.
 * \return Pointer to material properties, or NULL if not found.
 */
const MaterialProperties *get_material_properties_by_composition(const char *composition);

#ifdef __cplusplus
}
#endif
#endif /* OBSERVABLES_H */
