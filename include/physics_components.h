/** \file physics_components.h
 *  \brief Component wrappers for existing physics modules.
 *
 *  This header provides PhysicsComponent wrappers for the existing
 *  physics modules (beta, casimir, env, simulation) to integrate
 *  them with the unified physics framework.
 */
#ifndef PHYSICS_COMPONENTS_H
#define PHYSICS_COMPONENTS_H

#include "physics_framework.h"
#include "beta.h"
#include "casimir.h"
#include "env.h"
#include "simulation.h"
#include "observables.h"

#ifdef __cplusplus
extern "C" {
#endif

/* === QFT Beta Function Components === */

/** \brief φ⁴ theory first β-function coefficient component. */
extern const PhysicsComponent physics_beta1_component;

/** \brief φ⁴ theory second β-function coefficient component. */
extern const PhysicsComponent physics_beta2_component;

/** \brief φ⁴ theory anomalous dimension component. */
extern const PhysicsComponent physics_gamma_phi_component;

/* === Casimir Effect Components === */

/** \brief Casimir force (sphere-plate PFA) component. */
extern const PhysicsComponent physics_casimir_base_component;

/** \brief Casimir thermal correction component. */
extern const PhysicsComponent physics_casimir_thermal_component;

/** \brief Casimir modulated force component. */
extern const PhysicsComponent physics_casimir_modulated_component;

/* === Environment Components === */

/** \brief Environment parameter provider component. */
extern const PhysicsComponent physics_environment_component;

/* === Simulation Components === */

/** \brief Forward raytrace simulation component. */
extern const PhysicsComponent physics_forward_raytrace_component;

/** \brief Inverse retrieval simulation component. */
extern const PhysicsComponent physics_inverse_retrieve_component;

/** \brief Poisson solver component. */
extern const PhysicsComponent physics_poisson_solver_component;

/* === Material Properties Components === */

/** \brief Energy density observable component. */
extern const PhysicsComponent physics_energy_density_component;

/** \brief Enhanced energy density with material properties component. */
extern const PhysicsComponent physics_energy_density_enhanced_component;

/* === Composite Components === */

/** \brief Complete Casimir system (base + thermal + modulation) component. */
extern const PhysicsComponent physics_casimir_complete_component;

/** \brief QFT renormalization group analysis component. */
extern const PhysicsComponent physics_qft_rg_component;

/** \brief Complete physics demonstration combining QFT + Casimir + Environment. */
extern const PhysicsComponent physics_complete_demo_component;

/* === Component Registration === */

/** \brief Register all physics component wrappers with the framework. */
void physics_components_register_all(void);

/** \brief Create a complete physics demonstration context. */
PhysicsContext *physics_create_demo_context(void);

/** \brief Create a complete physics demonstration context with composite components. */
PhysicsContext *physics_create_composite_demo_context(void);

#ifdef __cplusplus
}
#endif

#endif /* PHYSICS_COMPONENTS_H */