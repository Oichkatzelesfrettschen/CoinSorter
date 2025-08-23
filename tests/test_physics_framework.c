/** \file test_physics_framework.c
 *  \brief Test suite for the unified physics framework.
 */
#include "physics_framework.h"
#include "physics_components.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static int test_basic_framework(void) {
    printf("Testing basic framework operations...\n");
    
    /* Test context creation and destruction */
    PhysicsContext *context = physics_context_create();
    assert(context != NULL);
    assert(context->enable_validation == true);
    assert(context->enable_dimensional_check == true);
    
    physics_context_destroy(context);
    
    printf("✓ Context creation/destruction\n");
    return 0;
}

static int test_component_registration(void) {
    printf("Testing component registration...\n");
    
    /* Register components */
    physics_components_register_all();
    
    /* Test component retrieval */
    const PhysicsComponent *beta1_comp = physics_framework_get_component("beta1");
    assert(beta1_comp != NULL);
    assert(strcmp(beta1_comp->name, "beta1") == 0);
    assert(beta1_comp->domain == PHYSICS_DOMAIN_QFT);
    
    const PhysicsComponent *casimir_comp = physics_framework_get_component("casimir_base");
    assert(casimir_comp != NULL);
    assert(strcmp(casimir_comp->name, "casimir_base") == 0);
    assert(casimir_comp->domain == PHYSICS_DOMAIN_CASIMIR);
    
    /* Test non-existent component */
    const PhysicsComponent *null_comp = physics_framework_get_component("nonexistent");
    assert(null_comp == NULL);
    
    printf("✓ Component registration and retrieval\n");
    return 0;
}

static int test_parameter_validation(void) {
    printf("Testing parameter validation...\n");
    
    /* Create a valid parameter */
    PhysicsParam valid_param = physics_param_create_double("test_param",
                                                           PHYSICS_DIM_LENGTH,
                                                           "m",
                                                           "Test parameter",
                                                           1e-6);
    
    char error_buffer[256];
    assert(physics_param_validate(&valid_param, error_buffer, sizeof(error_buffer)));
    
    /* Test parameter with invalid range (if we had set min/max values) */
    PhysicsParam range_param = valid_param;
    range_param.desc.min_value = 1e-5;
    range_param.desc.max_value = 1e-3;
    range_param.value.d = 1e-7; /* Below minimum */
    
    assert(!physics_param_validate(&range_param, error_buffer, sizeof(error_buffer)));
    
    printf("✓ Parameter validation\n");
    return 0;
}

static int test_dimensional_analysis(void) {
    printf("Testing dimensional analysis...\n");
    
    /* Test dimension compatibility */
    assert(physics_dimensions_compatible(PHYSICS_DIM_LENGTH, PHYSICS_DIM_LENGTH));
    assert(!physics_dimensions_compatible(PHYSICS_DIM_LENGTH, PHYSICS_DIM_MASS));
    
    /* Test dimension names */
    assert(strcmp(physics_dimension_name(PHYSICS_DIM_LENGTH), "length") == 0);
    assert(strcmp(physics_dimension_name(PHYSICS_DIM_FORCE), "force") == 0);
    assert(strcmp(physics_dimension_name(PHYSICS_DIM_DIMENSIONLESS), "dimensionless") == 0);
    
    printf("✓ Dimensional analysis\n");
    return 0;
}

static int test_physics_calculations(void) {
    printf("Testing physics calculations...\n");
    
    /* Register components */
    physics_components_register_all();
    
    /* Create demo context */
    PhysicsContext *context = physics_create_demo_context();
    assert(context != NULL);
    
    /* Validate context */
    char error_buffer[256];
    assert(physics_context_validate(context, error_buffer, sizeof(error_buffer)));
    
    /* Execute calculations */
    PhysicsResult *results = NULL;
    int ret = physics_context_execute(context, &results);
    assert(ret == 0);
    assert(results != NULL);
    
    /* Check results */
    bool found_beta1 = false, found_casimir = false;
    for (size_t i = 0; i < context->num_components; i++) {
        const PhysicsComponent *comp = context->components[i];
        const PhysicsResult *result = &results[i];
        
        assert(result->is_valid);
        
        if (strcmp(comp->name, "beta1") == 0) {
            assert(result->dimension == PHYSICS_DIM_DIMENSIONLESS);
            assert(result->value > 0.01 && result->value < 0.03); /* Reasonable range */
            found_beta1 = true;
        }
        
        if (strcmp(comp->name, "casimir_base") == 0) {
            assert(result->dimension == PHYSICS_DIM_FORCE);
            assert(result->value != 0.0); /* Should have some force value */
            found_casimir = true;
        }
    }
    
    assert(found_beta1);
    assert(found_casimir);
    
    free(results);
    physics_context_destroy(context);
    
    printf("✓ Physics calculations\n");
    return 0;
}

int main(void) {
    printf("=== Physics Framework Test Suite ===\n");
    
    int failed = 0;
    
    failed += test_basic_framework();
    failed += test_component_registration();
    failed += test_parameter_validation();
    failed += test_dimensional_analysis();
    failed += test_physics_calculations();
    
    if (failed == 0) {
        printf("\n✓ All physics framework tests passed!\n");
        printf("=== Framework Features Verified ===\n");
        printf("✓ Unified component architecture\n");
        printf("✓ Parameter validation and type checking\n");
        printf("✓ Dimensional analysis\n");
        printf("✓ Component registration and discovery\n");
        printf("✓ Context-based calculation execution\n");
        printf("✓ Error handling and validation\n");
        printf("✓ Self-describing components\n");
        return 0;
    } else {
        printf("\n✗ %d tests failed\n", failed);
        return 1;
    }
}