/** \file physics_components.c
 *  \brief Implementation of physics component wrappers.
 */
#include "physics_components.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

/* === Parameter Descriptors === */

/* Beta function parameter descriptors */
static const PhysicsParamDesc gamma_phi_params[] = {
    {
        .name = "coupling",
        .type = PHYSICS_PARAM_DOUBLE,
        .dimension = PHYSICS_DIM_DIMENSIONLESS,
        .units = "dimensionless",
        .description = "Coupling constant g",
        .required = true,
        .min_value = 0.0,
        .max_value = 10.0
    }
};

/* Casimir parameter descriptors */
static const PhysicsParamDesc casimir_base_params[] = {
    {
        .name = "radius",
        .type = PHYSICS_PARAM_DOUBLE,
        .dimension = PHYSICS_DIM_LENGTH,
        .units = "m",
        .description = "Sphere radius R",
        .required = true,
        .min_value = 1e-9,
        .max_value = 1e-3
    },
    {
        .name = "distance",
        .type = PHYSICS_PARAM_DOUBLE,
        .dimension = PHYSICS_DIM_LENGTH,
        .units = "m", 
        .description = "Plate distance d",
        .required = true,
        .min_value = 1e-12,
        .max_value = 1e-6
    }
};

static const PhysicsParamDesc casimir_thermal_params[] = {
    {
        .name = "radius",
        .type = PHYSICS_PARAM_DOUBLE,
        .dimension = PHYSICS_DIM_LENGTH,
        .units = "m",
        .description = "Sphere radius R",
        .required = true,
        .min_value = 1e-9,
        .max_value = 1e-3
    },
    {
        .name = "distance",
        .type = PHYSICS_PARAM_DOUBLE,
        .dimension = PHYSICS_DIM_LENGTH,
        .units = "m",
        .description = "Plate distance d", 
        .required = true,
        .min_value = 1e-12,
        .max_value = 1e-6
    },
    {
        .name = "temperature",
        .type = PHYSICS_PARAM_DOUBLE,
        .dimension = PHYSICS_DIM_TEMPERATURE,
        .units = "K",
        .description = "Temperature T",
        .required = true,
        .min_value = 0.1,
        .max_value = 1000.0
    }
};

/* === Calculation Functions === */

static PhysicsResult beta1_calculate(const PhysicsComponent *comp, 
                                      const PhysicsParam *params, 
                                      size_t num_params) {
    (void)comp; (void)params; (void)num_params;
    
    PhysicsResult result = {0};
    result.value = beta1();
    result.dimension = PHYSICS_DIM_DIMENSIONLESS;
    result.units = "dimensionless";
    result.uncertainty = 1e-15; /* Numerical precision */
    result.is_valid = true;
    result.error_msg = NULL;
    
    return result;
}

static PhysicsResult beta2_calculate(const PhysicsComponent *comp,
                                      const PhysicsParam *params,
                                      size_t num_params) {
    (void)comp; (void)params; (void)num_params;
    
    PhysicsResult result = {0};
    result.value = beta2();
    result.dimension = PHYSICS_DIM_DIMENSIONLESS;
    result.units = "dimensionless";
    result.uncertainty = 1e-15;
    result.is_valid = true;
    result.error_msg = NULL;
    
    return result;
}

static PhysicsResult gamma_phi_calculate(const PhysicsComponent *comp,
                                         const PhysicsParam *params,
                                         size_t num_params) {
    (void)comp;
    PhysicsResult result = {0};
    
    /* Find coupling parameter */
    double coupling = 1.0; /* default */
    for (size_t i = 0; i < num_params; i++) {
        if (strcmp(params[i].desc.name, "coupling") == 0) {
            coupling = params[i].value.d;
            break;
        }
    }
    
    result.value = gamma_phi(coupling);
    result.dimension = PHYSICS_DIM_DIMENSIONLESS;
    result.units = "dimensionless";
    result.uncertainty = 1e-15;
    result.is_valid = true;
    result.error_msg = NULL;
    
    return result;
}

static PhysicsResult casimir_base_calculate(const PhysicsComponent *comp,
                                            const PhysicsParam *params,
                                            size_t num_params) {
    (void)comp;
    PhysicsResult result = {0};
    
    double radius = 0.0, distance = 0.0;
    bool found_radius = false, found_distance = false;
    
    for (size_t i = 0; i < num_params; i++) {
        if (strcmp(params[i].desc.name, "radius") == 0) {
            radius = params[i].value.d;
            found_radius = true;
        } else if (strcmp(params[i].desc.name, "distance") == 0) {
            distance = params[i].value.d;
            found_distance = true;
        }
    }
    
    if (!found_radius || !found_distance) {
        result.is_valid = false;
        result.error_msg = "Missing required parameters";
        return result;
    }
    
    result.value = casimir_base(radius, distance);
    result.dimension = PHYSICS_DIM_FORCE;
    result.units = "N";
    result.uncertainty = fabs(result.value * 0.1); /* 10% uncertainty estimate */
    result.is_valid = true;
    result.error_msg = NULL;
    
    return result;
}

static PhysicsResult casimir_thermal_calculate(const PhysicsComponent *comp,
                                               const PhysicsParam *params,
                                               size_t num_params) {
    (void)comp;
    PhysicsResult result = {0};
    
    double radius = 0.0, distance = 0.0, temperature = 0.0;
    bool found_radius = false, found_distance = false, found_temp = false;
    
    for (size_t i = 0; i < num_params; i++) {
        if (strcmp(params[i].desc.name, "radius") == 0) {
            radius = params[i].value.d;
            found_radius = true;
        } else if (strcmp(params[i].desc.name, "distance") == 0) {
            distance = params[i].value.d;
            found_distance = true;
        } else if (strcmp(params[i].desc.name, "temperature") == 0) {
            temperature = params[i].value.d;
            found_temp = true;
        }
    }
    
    if (!found_radius || !found_distance || !found_temp) {
        result.is_valid = false;
        result.error_msg = "Missing required parameters";
        return result;
    }
    
    result.value = casimir_thermal(radius, distance, temperature);
    result.dimension = PHYSICS_DIM_FORCE;
    result.units = "N";
    result.uncertainty = fabs(result.value * 0.2); /* 20% uncertainty estimate */
    result.is_valid = true;
    result.error_msg = NULL;
    
    return result;
}

/* === Validation Functions === */

static bool basic_validation(const PhysicsComponent *comp,
                             const PhysicsParam *params,
                             size_t num_params,
                             char *error_buffer,
                             size_t buffer_size) {
    (void)comp;
    
    /* Validate each parameter */
    for (size_t i = 0; i < num_params; i++) {
        if (!physics_param_validate(&params[i], error_buffer, buffer_size)) {
            return false;
        }
    }
    
    return true;
}

/* === Component Definitions === */

const PhysicsComponent physics_beta1_component = {
    .name = "beta1",
    .description = "φ⁴ theory first β-function coefficient",
    .domain = PHYSICS_DOMAIN_QFT,
    .param_descs = NULL,
    .num_params = 0,
    .calculate = beta1_calculate,
    .validate = basic_validation,
    .dependencies = NULL,
    .num_dependencies = 0,
    .result_dimension = PHYSICS_DIM_DIMENSIONLESS,
    .result_units = "dimensionless"
};

const PhysicsComponent physics_beta2_component = {
    .name = "beta2", 
    .description = "φ⁴ theory second β-function coefficient",
    .domain = PHYSICS_DOMAIN_QFT,
    .param_descs = NULL,
    .num_params = 0,
    .calculate = beta2_calculate,
    .validate = basic_validation,
    .dependencies = NULL,
    .num_dependencies = 0,
    .result_dimension = PHYSICS_DIM_DIMENSIONLESS,
    .result_units = "dimensionless"
};

const PhysicsComponent physics_gamma_phi_component = {
    .name = "gamma_phi",
    .description = "φ⁴ theory anomalous dimension",
    .domain = PHYSICS_DOMAIN_QFT,
    .param_descs = gamma_phi_params,
    .num_params = sizeof(gamma_phi_params) / sizeof(gamma_phi_params[0]),
    .calculate = gamma_phi_calculate,
    .validate = basic_validation,
    .dependencies = NULL,
    .num_dependencies = 0,
    .result_dimension = PHYSICS_DIM_DIMENSIONLESS,
    .result_units = "dimensionless"
};

const PhysicsComponent physics_casimir_base_component = {
    .name = "casimir_base",
    .description = "Casimir force sphere-plate PFA",
    .domain = PHYSICS_DOMAIN_CASIMIR,
    .param_descs = casimir_base_params,
    .num_params = sizeof(casimir_base_params) / sizeof(casimir_base_params[0]),
    .calculate = casimir_base_calculate,
    .validate = basic_validation,
    .dependencies = NULL,
    .num_dependencies = 0,
    .result_dimension = PHYSICS_DIM_FORCE,
    .result_units = "N"
};

const PhysicsComponent physics_casimir_thermal_component = {
    .name = "casimir_thermal",
    .description = "Casimir thermal correction",
    .domain = PHYSICS_DOMAIN_CASIMIR,
    .param_descs = casimir_thermal_params,
    .num_params = sizeof(casimir_thermal_params) / sizeof(casimir_thermal_params[0]),
    .calculate = casimir_thermal_calculate,
    .validate = basic_validation,
    .dependencies = NULL,
    .num_dependencies = 0,
    .result_dimension = PHYSICS_DIM_FORCE,
    .result_units = "N"
};

/* === Registration Functions === */

void physics_components_register_all(void) {
    /* Register all component wrappers - external function in physics_framework.c */
    extern int physics_framework_register_component(const PhysicsComponent *component);
    
    physics_framework_register_component(&physics_beta1_component);
    physics_framework_register_component(&physics_beta2_component);
    physics_framework_register_component(&physics_gamma_phi_component);
    physics_framework_register_component(&physics_casimir_base_component);
    physics_framework_register_component(&physics_casimir_thermal_component);
    
    printf("[physics] Registered %d physics components\n", 5);
}

PhysicsContext *physics_create_demo_context(void) {
    PhysicsContext *context = physics_context_create();
    if (!context) return NULL;
    
    /* Add QFT components */
    physics_context_add_component(context, 
                                  (PhysicsComponent *)&physics_beta1_component, 
                                  NULL, 0);
    
    physics_context_add_component(context,
                                  (PhysicsComponent *)&physics_beta2_component,
                                  NULL, 0);
    
    /* Add gamma_phi with coupling parameter */
    PhysicsParam gamma_param = physics_param_create_double("coupling", 
                                                           PHYSICS_DIM_DIMENSIONLESS,
                                                           "dimensionless",
                                                           "Coupling constant",
                                                           1.0);
    physics_context_add_component(context,
                                  (PhysicsComponent *)&physics_gamma_phi_component,
                                  &gamma_param, 1);
    
    /* Add Casimir components */
    PhysicsParam casimir_params[] = {
        physics_param_create_double("radius", PHYSICS_DIM_LENGTH, "m", "Sphere radius", 5e-6),
        physics_param_create_double("distance", PHYSICS_DIM_LENGTH, "m", "Plate distance", 10e-9)
    };
    physics_context_add_component(context,
                                  (PhysicsComponent *)&physics_casimir_base_component,
                                  casimir_params, 2);
    
    PhysicsParam thermal_params[] = {
        physics_param_create_double("radius", PHYSICS_DIM_LENGTH, "m", "Sphere radius", 5e-6),
        physics_param_create_double("distance", PHYSICS_DIM_LENGTH, "m", "Plate distance", 10e-9),
        physics_param_create_double("temperature", PHYSICS_DIM_TEMPERATURE, "K", "Temperature", 293.0)
    };
    physics_context_add_component(context,
                                  (PhysicsComponent *)&physics_casimir_thermal_component,
                                  thermal_params, 3);
    
    return context;
}