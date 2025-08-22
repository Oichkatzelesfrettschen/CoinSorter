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

/* === Composite Component Calculations === */

static PhysicsResult qft_rg_calculate(const PhysicsComponent *comp,
                                       const PhysicsParam *params,
                                       size_t num_params) {
    (void)comp; (void)params; (void)num_params;
    
    PhysicsResult result = {0};
    
    /* Combine beta functions for RG analysis */
    double b1 = beta1();
    double b2 = beta2();
    double g = 1.0; /* default coupling */
    
    /* Extract coupling if provided */
    for (size_t i = 0; i < num_params; i++) {
        if (strcmp(params[i].desc.name, "coupling") == 0) {
            g = params[i].value.d;
            break;
        }
    }
    
    /* Compute effective beta function at this coupling */
    double beta_eff = b1 * g * g + b2 * g * g * g * g;
    double gamma = gamma_phi(g);
    
    /* Combine into RG "criticality metric" */
    result.value = fabs(beta_eff) + gamma;
    result.dimension = PHYSICS_DIM_DIMENSIONLESS;
    result.units = "dimensionless";
    result.uncertainty = fabs(result.value * 0.05);
    result.is_valid = true;
    result.error_msg = NULL;
    
    return result;
}

static PhysicsResult casimir_complete_calculate(const PhysicsComponent *comp,
                                                const PhysicsParam *params,
                                                size_t num_params) {
    (void)comp;
    PhysicsResult result = {0};
    
    double radius = 5e-6, distance = 10e-9, temperature = 293.0, anisotropy = 1.0, theta = 0.0;
    
    /* Extract parameters */
    for (size_t i = 0; i < num_params; i++) {
        const char *name = params[i].desc.name;
        if (strcmp(name, "radius") == 0) {
            radius = params[i].value.d;
        } else if (strcmp(name, "distance") == 0) {
            distance = params[i].value.d;
        } else if (strcmp(name, "temperature") == 0) {
            temperature = params[i].value.d;
        } else if (strcmp(name, "anisotropy") == 0) {
            anisotropy = params[i].value.d;
        } else if (strcmp(name, "theta") == 0) {
            theta = params[i].value.d;
        }
    }
    
    /* Compose complete Casimir calculation */
    double F_base = casimir_base(radius, distance);
    double F_thermal = casimir_thermal(radius, distance, temperature);
    double F_total = casimir_modulated(F_base, F_thermal, anisotropy, theta);
    
    result.value = F_total;
    result.dimension = PHYSICS_DIM_FORCE;
    result.units = "N";
    result.uncertainty = fabs(result.value * 0.15); /* 15% combined uncertainty */
    result.is_valid = true;
    result.error_msg = NULL;
    
    return result;
}

static PhysicsResult complete_demo_calculate(const PhysicsComponent *comp,
                                             const PhysicsParam *params,
                                             size_t num_params) {
    (void)comp;
    PhysicsResult result = {0};
    
    /* This demonstrates recursive composition by combining multiple physics domains */
    
    /* QFT contribution */
    double qft_metric = 0.0;
    PhysicsResult qft_result = qft_rg_calculate(comp, params, num_params);
    if (qft_result.is_valid) {
        qft_metric = qft_result.value;
    }
    
    /* Casimir contribution */
    double casimir_force = 0.0;
    PhysicsResult casimir_result = casimir_complete_calculate(comp, params, num_params);
    if (casimir_result.is_valid) {
        casimir_force = fabs(casimir_result.value);
    }
    
    /* Environment scaling (from parameter if provided) */
    double env_gravity = 9.807; /* default Earth */
    for (size_t i = 0; i < num_params; i++) {
        if (strcmp(params[i].desc.name, "gravity") == 0) {
            env_gravity = params[i].value.d;
            break;
        }
    }
    
    /* Compose a "unified physics metric" that combines all domains */
    /* This is a demonstration of how different physics domains can be composed */
    double unified_metric = (qft_metric * 1e6) + (casimir_force * 1e12) + (env_gravity / 10.0);
    
    result.value = unified_metric;
    result.dimension = PHYSICS_DIM_DIMENSIONLESS;
    result.units = "composite";
    result.uncertainty = unified_metric * 0.1;
    result.is_valid = true;
    result.error_msg = NULL;
    
    return result;
}

static const PhysicsParamDesc casimir_complete_params[] = {
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
        .required = false,
        .min_value = 0.1,
        .max_value = 1000.0
    },
    {
        .name = "anisotropy",
        .type = PHYSICS_PARAM_DOUBLE,
        .dimension = PHYSICS_DIM_DIMENSIONLESS,
        .units = "dimensionless",
        .description = "Anisotropy parameter",
        .required = false,
        .min_value = 0.0,
        .max_value = 10.0
    },
    {
        .name = "theta",
        .type = PHYSICS_PARAM_DOUBLE,
        .dimension = PHYSICS_DIM_DIMENSIONLESS,
        .units = "rad",
        .description = "Modulation angle",
        .required = false,
        .min_value = 0.0,
        .max_value = 6.28 /* 2π */
    }
};

static const PhysicsParamDesc complete_demo_params[] = {
    {
        .name = "coupling",
        .type = PHYSICS_PARAM_DOUBLE,
        .dimension = PHYSICS_DIM_DIMENSIONLESS,
        .units = "dimensionless",
        .description = "QFT coupling constant",
        .required = false,
        .min_value = 0.0,
        .max_value = 10.0
    },
    {
        .name = "radius",
        .type = PHYSICS_PARAM_DOUBLE,
        .dimension = PHYSICS_DIM_LENGTH,
        .units = "m",
        .description = "Sphere radius",
        .required = false,
        .min_value = 1e-9,
        .max_value = 1e-3
    },
    {
        .name = "distance",
        .type = PHYSICS_PARAM_DOUBLE,
        .dimension = PHYSICS_DIM_LENGTH,
        .units = "m",
        .description = "Plate distance",
        .required = false,
        .min_value = 1e-12,
        .max_value = 1e-6
    },
    {
        .name = "temperature",
        .type = PHYSICS_PARAM_DOUBLE,
        .dimension = PHYSICS_DIM_TEMPERATURE,
        .units = "K",
        .description = "Environment temperature",
        .required = false,
        .min_value = 0.1,
        .max_value = 1000.0
    },
    {
        .name = "gravity",
        .type = PHYSICS_PARAM_DOUBLE,
        .dimension = PHYSICS_DIM_DIMENSIONLESS,
        .units = "m/s^2",
        .description = "Gravitational acceleration",
        .required = false,
        .min_value = 0.1,
        .max_value = 100.0
    }
};

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

/* === Composite Component Definitions === */

const PhysicsComponent physics_qft_rg_component = {
    .name = "qft_rg",
    .description = "QFT renormalization group analysis",
    .domain = PHYSICS_DOMAIN_COMPOSITE,
    .param_descs = gamma_phi_params,
    .num_params = sizeof(gamma_phi_params) / sizeof(gamma_phi_params[0]),
    .calculate = qft_rg_calculate,
    .validate = basic_validation,
    .dependencies = NULL,
    .num_dependencies = 0,
    .result_dimension = PHYSICS_DIM_DIMENSIONLESS,
    .result_units = "dimensionless"
};

const PhysicsComponent physics_casimir_complete_component = {
    .name = "casimir_complete",
    .description = "Complete Casimir system (base + thermal + modulation)",
    .domain = PHYSICS_DOMAIN_COMPOSITE,
    .param_descs = casimir_complete_params,
    .num_params = sizeof(casimir_complete_params) / sizeof(casimir_complete_params[0]),
    .calculate = casimir_complete_calculate,
    .validate = basic_validation,
    .dependencies = NULL,
    .num_dependencies = 0,
    .result_dimension = PHYSICS_DIM_FORCE,
    .result_units = "N"
};

const PhysicsComponent physics_complete_demo_component = {
    .name = "complete_demo",
    .description = "Complete physics demonstration (QFT + Casimir + Environment)",
    .domain = PHYSICS_DOMAIN_COMPOSITE,
    .param_descs = complete_demo_params,
    .num_params = sizeof(complete_demo_params) / sizeof(complete_demo_params[0]),
    .calculate = complete_demo_calculate,
    .validate = basic_validation,
    .dependencies = NULL,
    .num_dependencies = 0,
    .result_dimension = PHYSICS_DIM_DIMENSIONLESS,
    .result_units = "composite"
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
    
    /* Register composite components */
    physics_framework_register_component(&physics_qft_rg_component);
    physics_framework_register_component(&physics_casimir_complete_component);
    physics_framework_register_component(&physics_complete_demo_component);
    
    printf("[physics] Registered %d physics components\n", 8);
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

PhysicsContext *physics_create_composite_demo_context(void) {
    PhysicsContext *context = physics_context_create();
    if (!context) return NULL;
    
    /* Add QFT RG composite component */
    PhysicsParam qft_rg_param = physics_param_create_double("coupling", 
                                                            PHYSICS_DIM_DIMENSIONLESS,
                                                            "dimensionless",
                                                            "QFT coupling constant",
                                                            1.5);
    physics_context_add_component(context,
                                  (PhysicsComponent *)&physics_qft_rg_component,
                                  &qft_rg_param, 1);
    
    /* Add complete Casimir composite component */
    PhysicsParam casimir_complete_params[] = {
        physics_param_create_double("radius", PHYSICS_DIM_LENGTH, "m", "Sphere radius", 5e-6),
        physics_param_create_double("distance", PHYSICS_DIM_LENGTH, "m", "Plate distance", 10e-9),
        physics_param_create_double("temperature", PHYSICS_DIM_TEMPERATURE, "K", "Temperature", 293.0),
        physics_param_create_double("anisotropy", PHYSICS_DIM_DIMENSIONLESS, "dimensionless", "Anisotropy", 2.0),
        physics_param_create_double("theta", PHYSICS_DIM_DIMENSIONLESS, "rad", "Modulation angle", 1.57) /* π/2 */
    };
    physics_context_add_component(context,
                                  (PhysicsComponent *)&physics_casimir_complete_component,
                                  casimir_complete_params, 5);
    
    /* Add complete physics demo component that combines everything */
    PhysicsParam complete_demo_params[] = {
        physics_param_create_double("coupling", PHYSICS_DIM_DIMENSIONLESS, "dimensionless", "QFT coupling", 1.2),
        physics_param_create_double("radius", PHYSICS_DIM_LENGTH, "m", "Sphere radius", 8e-6),
        physics_param_create_double("distance", PHYSICS_DIM_LENGTH, "m", "Plate distance", 15e-9),
        physics_param_create_double("temperature", PHYSICS_DIM_TEMPERATURE, "K", "Temperature", 300.0),
        physics_param_create_double("gravity", PHYSICS_DIM_DIMENSIONLESS, "m/s^2", "Gravity", 3.71) /* Mars gravity */
    };
    physics_context_add_component(context,
                                  (PhysicsComponent *)&physics_complete_demo_component,
                                  complete_demo_params, 5);
    
    return context;
}