/** \file physics_framework.c
 *  \brief Implementation of unified physics framework.
 */
#include "physics_framework.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

/* Maximum number of registered components */
#define MAX_COMPONENTS 32
#define MAX_ERROR_MSG 256

/* Component registry */
static const PhysicsComponent *registered_components[MAX_COMPONENTS];
static size_t num_registered_components = 0;

/* === Context Management === */

PhysicsContext *physics_context_create(void) {
    PhysicsContext *context = (PhysicsContext *)calloc(1, sizeof(PhysicsContext));
    if (!context) return NULL;
    
    context->enable_validation = true;
    context->enable_dimensional_check = true;
    return context;
}

void physics_context_destroy(PhysicsContext *context) {
    if (!context) return;
    
    if (context->components) {
        free(context->components);
    }
    if (context->param_sets) {
        for (size_t i = 0; i < context->num_components; i++) {
            free(context->param_sets[i]);
        }
        free(context->param_sets);
    }
    if (context->param_counts) {
        free(context->param_counts);
    }
    free(context);
}

int physics_context_add_component(PhysicsContext *context, 
                                   PhysicsComponent *component,
                                   PhysicsParam *params,
                                   size_t num_params) {
    if (!context || !component) return -1;
    
    /* Resize arrays if needed */
    size_t new_size = context->num_components + 1;
    
    PhysicsComponent **new_components = (PhysicsComponent **)realloc(
        context->components, new_size * sizeof(PhysicsComponent *));
    if (!new_components) return -1;
    context->components = new_components;
    
    PhysicsParam **new_param_sets = (PhysicsParam **)realloc(
        context->param_sets, new_size * sizeof(PhysicsParam *));
    if (!new_param_sets) return -1;
    context->param_sets = new_param_sets;
    
    size_t *new_param_counts = (size_t *)realloc(
        context->param_counts, new_size * sizeof(size_t));
    if (!new_param_counts) return -1;
    context->param_counts = new_param_counts;
    
    /* Copy parameters */
    PhysicsParam *param_copy = NULL;
    if (num_params > 0 && params) {
        param_copy = (PhysicsParam *)malloc(num_params * sizeof(PhysicsParam));
        if (!param_copy) return -1;
        memcpy(param_copy, params, num_params * sizeof(PhysicsParam));
    }
    
    /* Add to context */
    context->components[context->num_components] = component;
    context->param_sets[context->num_components] = param_copy;
    context->param_counts[context->num_components] = num_params;
    context->num_components++;
    
    return 0;
}

int physics_context_execute(PhysicsContext *context, PhysicsResult **results) {
    if (!context || !results) return -1;
    
    /* Allocate results array */
    *results = (PhysicsResult *)calloc(context->num_components, sizeof(PhysicsResult));
    if (!*results) return -1;
    
    /* Validate context if enabled */
    if (context->enable_validation) {
        char error_buffer[MAX_ERROR_MSG];
        if (!physics_context_validate(context, error_buffer, sizeof(error_buffer))) {
            printf("[physics] Validation failed: %s\n", error_buffer);
            free(*results);
            *results = NULL;
            return -1;
        }
    }
    
    /* Execute each component */
    for (size_t i = 0; i < context->num_components; i++) {
        const PhysicsComponent *comp = context->components[i];
        PhysicsParam *params = context->param_sets[i];
        size_t num_params = context->param_counts[i];
        
        if (comp && comp->calculate) {
            (*results)[i] = comp->calculate(comp, params, num_params);
        } else {
            (*results)[i].is_valid = false;
            (*results)[i].error_msg = "No calculation function";
        }
    }
    
    return 0;
}

bool physics_context_validate(const PhysicsContext *context, 
                               char *error_buffer, 
                               size_t buffer_size) {
    if (!context) {
        snprintf(error_buffer, buffer_size, "Invalid context");
        return false;
    }
    
    for (size_t i = 0; i < context->num_components; i++) {
        const PhysicsComponent *comp = context->components[i];
        PhysicsParam *params = context->param_sets[i];
        size_t num_params = context->param_counts[i];
        
        /* Validate component parameters */
        if (comp && comp->validate) {
            if (!comp->validate(comp, params, num_params, error_buffer, buffer_size)) {
                return false;
            }
        }
        
        /* Check required parameters */
        for (size_t j = 0; j < comp->num_params; j++) {
            const PhysicsParamDesc *desc = &comp->param_descs[j];
            if (desc->required) {
                bool found = false;
                for (size_t k = 0; k < num_params; k++) {
                    if (strcmp(params[k].desc.name, desc->name) == 0) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    snprintf(error_buffer, buffer_size, 
                             "Required parameter '%s' missing for component '%s'",
                             desc->name, comp->name);
                    return false;
                }
            }
        }
    }
    
    return true;
}

/* === Parameter Management === */

PhysicsParam physics_param_create_double(const char *name, 
                                          PhysicsDimension dimension,
                                          const char *units,
                                          const char *description,
                                          double value) {
    PhysicsParam param;
    memset(&param, 0, sizeof(param));
    
    param.desc.name = name;
    param.desc.type = PHYSICS_PARAM_DOUBLE;
    param.desc.dimension = dimension;
    param.desc.units = units;
    param.desc.description = description;
    param.desc.required = true;
    param.desc.min_value = -INFINITY;
    param.desc.max_value = INFINITY;
    
    param.value.d = value;
    param.is_set = true;
    
    return param;
}

bool physics_param_set_value(PhysicsParam *param, PhysicsParamValue value) {
    if (!param) return false;
    
    param->value = value;
    param->is_set = true;
    return true;
}

bool physics_param_validate(const PhysicsParam *param, 
                             char *error_buffer, 
                             size_t buffer_size) {
    if (!param) {
        snprintf(error_buffer, buffer_size, "Invalid parameter");
        return false;
    }
    
    if (param->desc.required && !param->is_set) {
        snprintf(error_buffer, buffer_size, 
                 "Required parameter '%s' not set", param->desc.name);
        return false;
    }
    
    /* Validate numeric ranges */
    if (param->desc.type == PHYSICS_PARAM_DOUBLE && param->is_set) {
        double val = param->value.d;
        if (val < param->desc.min_value || val > param->desc.max_value) {
            snprintf(error_buffer, buffer_size,
                     "Parameter '%s' value %.6g outside range [%.6g, %.6g]",
                     param->desc.name, val, param->desc.min_value, param->desc.max_value);
            return false;
        }
    }
    
    return true;
}

/* === Dimensional Analysis === */

bool physics_dimensions_compatible(PhysicsDimension dim1, PhysicsDimension dim2) {
    return dim1 == dim2;
}

const char *physics_dimension_name(PhysicsDimension dim) {
    switch (dim) {
        case PHYSICS_DIM_DIMENSIONLESS: return "dimensionless";
        case PHYSICS_DIM_LENGTH: return "length";
        case PHYSICS_DIM_MASS: return "mass";
        case PHYSICS_DIM_TIME: return "time";
        case PHYSICS_DIM_FORCE: return "force";
        case PHYSICS_DIM_ENERGY: return "energy";
        case PHYSICS_DIM_TEMPERATURE: return "temperature";
        case PHYSICS_DIM_PRESSURE: return "pressure";
        case PHYSICS_DIM_FREQUENCY: return "frequency";
        default: return "unknown";
    }
}

/* === Component Registration === */

void physics_framework_register_builtin_components(void) {
    /* This will be implemented as we adapt existing components */
    printf("[physics] Built-in components registration placeholder\n");
}

const PhysicsComponent *physics_framework_get_component(const char *name) {
    if (!name) return NULL;
    
    for (size_t i = 0; i < num_registered_components; i++) {
        if (registered_components[i] && 
            strcmp(registered_components[i]->name, name) == 0) {
            return registered_components[i];
        }
    }
    return NULL;
}

void physics_framework_list_components(void) {
    printf("[physics] Registered components (%zu):\n", num_registered_components);
    for (size_t i = 0; i < num_registered_components; i++) {
        const PhysicsComponent *comp = registered_components[i];
        if (comp) {
            printf("  %s: %s (domain: %s)\n", 
                   comp->name, comp->description, 
                   (comp->domain == PHYSICS_DOMAIN_QFT) ? "QFT" :
                   (comp->domain == PHYSICS_DOMAIN_CASIMIR) ? "Casimir" :
                   (comp->domain == PHYSICS_DOMAIN_ENVIRONMENT) ? "Environment" :
                   (comp->domain == PHYSICS_DOMAIN_SIMULATION) ? "Simulation" :
                   (comp->domain == PHYSICS_DOMAIN_MATERIALS) ? "Materials" : "Other");
        }
    }
}

/* === Composition and Validation === */

bool physics_components_composable(const PhysicsComponent **components,
                                   size_t num_components,
                                   char *error_buffer,
                                   size_t buffer_size) {
    if (!components || num_components == 0) {
        snprintf(error_buffer, buffer_size, "No components provided");
        return false;
    }
    
    /* Check for dependency cycles and missing dependencies */
    for (size_t i = 0; i < num_components; i++) {
        const PhysicsComponent *comp = components[i];
        if (!comp) {
            snprintf(error_buffer, buffer_size, "Component %zu is NULL", i);
            return false;
        }
        
        /* Check dependencies are available */
        for (size_t j = 0; j < comp->num_dependencies; j++) {
            const PhysicsComponent *dep = comp->dependencies[j];
            bool found = false;
            for (size_t k = 0; k < num_components; k++) {
                if (components[k] == dep) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                snprintf(error_buffer, buffer_size,
                         "Component '%s' requires dependency '%s' which is not available",
                         comp->name, dep ? dep->name : "(null)");
                return false;
            }
        }
    }
    
    return true;
}

int physics_resolve_dependencies(PhysicsComponent **components,
                                 size_t num_components,
                                 PhysicsComponent ***ordered_components) {
    if (!components || !ordered_components) return -1;
    
    /* Simple topological sort - for now just return original order */
    *ordered_components = (PhysicsComponent **)malloc(num_components * sizeof(PhysicsComponent *));
    if (!*ordered_components) return -1;
    
    memcpy(*ordered_components, components, num_components * sizeof(PhysicsComponent *));
    return 0;
}

/* === Utility Functions === */

/** \brief Register a component in the global registry. */
int physics_framework_register_component(const PhysicsComponent *component) {
    if (!component || num_registered_components >= MAX_COMPONENTS) {
        return -1;
    }
    
    registered_components[num_registered_components++] = component;
    return 0;
}