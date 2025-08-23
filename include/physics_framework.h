/** \file physics_framework.h
 *  \brief Unified physics framework for composable, introspectable physics components.
 *
 *  This framework transforms the experimental physics modules into a recursively
 *  complete system where components can be composed, validated, and executed
 *  as complete computational graphs.
 */
#ifndef PHYSICS_FRAMEWORK_H
#define PHYSICS_FRAMEWORK_H

#include "physics_constants.h"
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** \brief Physics domain enumeration for component classification. */
typedef enum {
    PHYSICS_DOMAIN_QFT,          /**< Quantum field theory */
    PHYSICS_DOMAIN_CASIMIR,      /**< Casimir effect */
    PHYSICS_DOMAIN_ENVIRONMENT,  /**< Environmental parameters */
    PHYSICS_DOMAIN_SIMULATION,   /**< Numerical simulation */
    PHYSICS_DOMAIN_MATERIALS,    /**< Material properties */
    PHYSICS_DOMAIN_COMPOSITE     /**< Composite of multiple domains */
} PhysicsDomain;

/** \brief Parameter type enumeration for type checking. */
typedef enum {
    PHYSICS_PARAM_DOUBLE,        /**< Double precision floating point */
    PHYSICS_PARAM_INT,           /**< Integer */
    PHYSICS_PARAM_STRING,        /**< String */
    PHYSICS_PARAM_POINTER        /**< Generic pointer */
} PhysicsParamType;

/** \brief Physical dimension enumeration for dimensional analysis. */
typedef enum {
    PHYSICS_DIM_DIMENSIONLESS,   /**< Dimensionless quantity */
    PHYSICS_DIM_LENGTH,          /**< Length [L] */
    PHYSICS_DIM_MASS,            /**< Mass [M] */
    PHYSICS_DIM_TIME,            /**< Time [T] */
    PHYSICS_DIM_FORCE,           /**< Force [MLT⁻²] */
    PHYSICS_DIM_ENERGY,          /**< Energy [ML²T⁻²] */
    PHYSICS_DIM_TEMPERATURE,     /**< Temperature [K] */
    PHYSICS_DIM_PRESSURE,        /**< Pressure [ML⁻¹T⁻²] */
    PHYSICS_DIM_FREQUENCY        /**< Frequency [T⁻¹] */
} PhysicsDimension;

/** \brief Parameter descriptor for introspection. */
typedef struct {
    const char *name;            /**< Parameter name */
    PhysicsParamType type;       /**< Parameter type */
    PhysicsDimension dimension;  /**< Physical dimension */
    const char *units;           /**< Unit description */
    const char *description;     /**< Human-readable description */
    bool required;               /**< Whether parameter is required */
    double min_value;            /**< Minimum allowed value (for numerics) */
    double max_value;            /**< Maximum allowed value (for numerics) */
} PhysicsParamDesc;

/** \brief Generic parameter value container. */
typedef union {
    double d;                    /**< Double value */
    int i;                       /**< Integer value */
    const char *s;               /**< String value */
    void *p;                     /**< Pointer value */
} PhysicsParamValue;

/** \brief Parameter instance with value. */
typedef struct {
    PhysicsParamDesc desc;       /**< Parameter descriptor */
    PhysicsParamValue value;     /**< Parameter value */
    bool is_set;                 /**< Whether value has been set */
} PhysicsParam;

/** \brief Result of physics calculation with metadata. */
typedef struct {
    double value;                /**< Calculated value */
    PhysicsDimension dimension;  /**< Physical dimension of result */
    const char *units;           /**< Unit description */
    double uncertainty;          /**< Estimated uncertainty */
    bool is_valid;               /**< Whether calculation succeeded */
    const char *error_msg;       /**< Error message if calculation failed */
} PhysicsResult;

/** \brief Forward declaration of physics component. */
typedef struct PhysicsComponent PhysicsComponent;

/** \brief Function pointer for physics calculations. */
typedef PhysicsResult (*PhysicsCalculationFunc)(const PhysicsComponent *comp, 
                                                 const PhysicsParam *params, 
                                                 size_t num_params);

/** \brief Function pointer for component validation. */
typedef bool (*PhysicsValidationFunc)(const PhysicsComponent *comp,
                                       const PhysicsParam *params,
                                       size_t num_params,
                                       char *error_buffer,
                                       size_t buffer_size);

/** \brief Physics component descriptor. */
struct PhysicsComponent {
    const char *name;                        /**< Component name */
    const char *description;                 /**< Component description */
    PhysicsDomain domain;                    /**< Physics domain */
    const PhysicsParamDesc *param_descs;     /**< Parameter descriptors */
    size_t num_params;                       /**< Number of parameters */
    PhysicsCalculationFunc calculate;        /**< Calculation function */
    PhysicsValidationFunc validate;          /**< Validation function */
    const PhysicsComponent **dependencies;   /**< Required dependencies */
    size_t num_dependencies;                 /**< Number of dependencies */
    PhysicsDimension result_dimension;       /**< Expected result dimension */
    const char *result_units;                /**< Expected result units */
};

/** \brief Physics calculation context for composing multiple components. */
typedef struct {
    PhysicsComponent **components;           /**< Array of components */
    size_t num_components;                   /**< Number of components */
    PhysicsParam **param_sets;               /**< Parameter sets for each component */
    size_t *param_counts;                    /**< Parameter count for each component */
    bool enable_validation;                  /**< Whether to perform validation */
    bool enable_dimensional_check;           /**< Whether to check dimensions */
} PhysicsContext;

/* === Framework Core Functions === */

/** \brief Create a new physics calculation context. */
PhysicsContext *physics_context_create(void);

/** \brief Destroy a physics context and free resources. */
void physics_context_destroy(PhysicsContext *context);

/** \brief Add a component to the calculation context. */
int physics_context_add_component(PhysicsContext *context, 
                                   PhysicsComponent *component,
                                   PhysicsParam *params,
                                   size_t num_params);

/** \brief Execute all components in dependency order. */
int physics_context_execute(PhysicsContext *context, PhysicsResult **results);

/** \brief Validate all components and their parameter sets. */
bool physics_context_validate(const PhysicsContext *context, 
                               char *error_buffer, 
                               size_t buffer_size);

/* === Parameter Management === */

/** \brief Create a parameter with given descriptor and value. */
PhysicsParam physics_param_create_double(const char *name, 
                                          PhysicsDimension dimension,
                                          const char *units,
                                          const char *description,
                                          double value);

/** \brief Set parameter value with type checking. */
bool physics_param_set_value(PhysicsParam *param, PhysicsParamValue value);

/** \brief Validate parameter value against constraints. */
bool physics_param_validate(const PhysicsParam *param, 
                             char *error_buffer, 
                             size_t buffer_size);

/* === Dimensional Analysis === */

/** \brief Check if two dimensions are compatible. */
bool physics_dimensions_compatible(PhysicsDimension dim1, PhysicsDimension dim2);

/** \brief Get dimension name for human-readable output. */
const char *physics_dimension_name(PhysicsDimension dim);

/* === Component Registration === */

/** \brief Register built-in physics components with the framework. */
void physics_framework_register_builtin_components(void);

/** \brief Get component by name from registry. */
const PhysicsComponent *physics_framework_get_component(const char *name);

/** \brief List all registered components. */
void physics_framework_list_components(void);

/* === Composition and Validation === */

/** \brief Check if components can be composed together. */
bool physics_components_composable(const PhysicsComponent **components,
                                   size_t num_components,
                                   char *error_buffer,
                                   size_t buffer_size);

/** \brief Resolve dependency order for component execution. */
int physics_resolve_dependencies(PhysicsComponent **components,
                                 size_t num_components,
                                 PhysicsComponent ***ordered_components);

#ifdef __cplusplus
}
#endif

#endif /* PHYSICS_FRAMEWORK_H */