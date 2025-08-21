/** 
 * \file test_observables.c
 * \brief Test suite for material properties and observables functionality.
 *
 * Tests the comprehensive material properties database and physics observables
 * to ensure accurate material property lookup and calculations.
 */
#include "observables.h"
#include "coins.h"
#include <stdio.h>
#include <math.h>
#include <string.h>

#define TEST_TOLERANCE 1e-6

static int test_count = 0;
static int test_passed = 0;

void assert_test(int condition, const char *test_name) {
  test_count++;
  if (condition) {
    test_passed++;
    printf("PASS: %s\n", test_name);
  } else {
    printf("FAIL: %s\n", test_name);
  }
}

void assert_double_close(double actual, double expected, const char *test_name) {
  test_count++;
  if (fabs(actual - expected) < TEST_TOLERANCE) {
    test_passed++;
    printf("PASS: %s (%.6f ≈ %.6f)\n", test_name, actual, expected);
  } else {
    printf("FAIL: %s (%.6f != %.6f)\n", test_name, actual, expected);
  }
}

int main(void) {
  printf("=== Material Properties and Observables Test Suite ===\n\n");

  /* Test material property lookup by composition */
  printf("--- Material Property Lookup Tests ---\n");
  
  const MaterialProperties *cu_props = get_material_properties_by_composition("8.33% Ni bal Cu (clad)");
  assert_test(cu_props != NULL, "Cupronickel clad lookup");
  if (cu_props) {
    assert_double_close(cu_props->electrical_conductivity, 3.2e6, "Cupronickel conductivity");
    assert_test(strcmp(cu_props->material_class, "Cupronickel") == 0, "Cupronickel class");
  }
  
  const MaterialProperties *steel_props = get_material_properties_by_composition("Multi-ply Ni plated steel");
  assert_test(steel_props != NULL, "Ni-plated steel lookup");
  if (steel_props) {
    assert_test(steel_props->relative_permeability > 100.0, "Steel ferromagnetic");
    assert_test(strcmp(steel_props->material_class, "Nickel-plated Steel") == 0, "Steel class");
  }
  
  const MaterialProperties *nordic_props = get_material_properties_by_composition("Nordic gold");
  assert_test(nordic_props != NULL, "Nordic gold lookup");
  if (nordic_props) {
    assert_test(nordic_props->relative_permeability < 1.0, "Nordic gold diamagnetic");
    assert_test(strcmp(nordic_props->material_class, "Nordic Gold") == 0, "Nordic gold class");
  }
  
  /* Test with actual coin specs */
  printf("\n--- Coin Spec Integration Tests ---\n");
  
  const CoinSystem *usd = get_coin_system("usd");
  assert_test(usd != NULL, "USD system available");
  
  if (usd && usd->ncoins > 0) {
    const CoinSpec *quarter = &usd->coins[0];  /* First coin should be quarter */
    const MaterialProperties *quarter_props = get_material_properties(quarter);
    assert_test(quarter_props != NULL, "Quarter material properties lookup");
    
    if (quarter_props) {
      assert_test(quarter_props->electrical_conductivity > 1e6, "Quarter has good conductivity");
    }
  }
  
  /* Test basic energy density functions */
  printf("\n--- Energy Density Function Tests ---\n");
  
  double basic_energy = observable_energy_density(1.0, 1.0);
  assert_double_close(basic_energy, 1.0, "Basic energy density (1,1)");
  
  double zero_energy = observable_energy_density(0.0, 0.0);
  assert_double_close(zero_energy, 0.0, "Zero energy density");
  
  /* Test enhanced energy density with material properties */
  if (cu_props) {
    double enhanced_energy = observable_energy_density_enhanced(1.0, 1.0, cu_props);
    assert_test(enhanced_energy > basic_energy, "Enhanced energy > basic energy");
    
    double enhanced_zero = observable_energy_density_enhanced(0.0, 0.0, cu_props);
    assert_double_close(enhanced_zero, 0.0, "Enhanced zero energy");
  }
  
  /* Test electromagnetic energy density */
  printf("\n--- Electromagnetic Energy Tests ---\n");
  
  if (cu_props) {
    double em_energy = observable_em_energy_density(1000.0, 0.001, cu_props);
    assert_test(em_energy > 0.0, "EM energy density positive");
    
    double em_zero = observable_em_energy_density(0.0, 0.0, cu_props);
    assert_double_close(em_zero, 0.0, "Zero EM energy");
  }
  
  /* Test thermal diffusivity */
  printf("\n--- Thermal Property Tests ---\n");
  
  if (cu_props) {
    double thermal_diff = observable_thermal_diffusivity(cu_props);
    assert_test(thermal_diff > 0.0, "Thermal diffusivity positive");
    
    /* Material should have reasonable thermal diffusivity */
    assert_test(thermal_diff > 1e-6, "Material reasonable thermal diffusivity");
  }
  
  /* Test skin depth calculation */
  printf("\n--- Electromagnetic Skin Depth Tests ---\n");
  
  if (cu_props) {
    double skin_60hz = observable_skin_depth(60.0, cu_props);  /* 60 Hz AC */
    assert_test(skin_60hz > 0.001, "60 Hz skin depth reasonable");
    
    double skin_1mhz = observable_skin_depth(1e6, cu_props);   /* 1 MHz */
    assert_test(skin_1mhz < skin_60hz, "Higher frequency = smaller skin depth");
  }
  
  /* Test acoustic impedance */
  printf("\n--- Acoustic Property Tests ---\n");
  
  if (cu_props) {
    double acoustic_z = observable_acoustic_impedance(cu_props);
    assert_test(acoustic_z > 1e6, "Acoustic impedance reasonable magnitude");
  }
  
  /* Test edge cases and error handling */
  printf("\n--- Error Handling Tests ---\n");
  
  const MaterialProperties *null_props = get_material_properties_by_composition(NULL);
  assert_test(null_props == NULL, "NULL composition returns NULL");
  
  const MaterialProperties *unknown_props = get_material_properties_by_composition("Unobtainium");
  assert_test(unknown_props == NULL, "Unknown material returns NULL");
  
  double enhanced_null = observable_energy_density_enhanced(1.0, 1.0, NULL);
  assert_double_close(enhanced_null, basic_energy, "Enhanced with NULL properties");
  
  double thermal_null = observable_thermal_diffusivity(NULL);
  assert_double_close(thermal_null, 0.0, "Thermal diffusivity with NULL");
  
  double skin_zero_freq = observable_skin_depth(0.0, cu_props);
  assert_test(isinf(skin_zero_freq), "Zero frequency gives infinite skin depth");
  
  double acoustic_null = observable_acoustic_impedance(NULL);
  assert_double_close(acoustic_null, 0.0, "Acoustic impedance with NULL");
  
  /* Test material property ranges and physical reasonableness */
  printf("\n--- Physical Reasonableness Tests ---\n");
  
  const MaterialProperties *al_props = get_material_properties_by_composition("Aluminum");
  if (al_props) {
    assert_test(al_props->density < cu_props->density, "Aluminum lighter than copper");
    assert_test(al_props->electrical_conductivity > 1e7, "Aluminum good conductor");
    assert_test(fabs(al_props->magnetic_susceptibility) < 1e-4, "Aluminum weakly magnetic");
  }
  
  /* Summary */
  printf("\n=== Test Results ===\n");
  printf("Tests passed: %d/%d\n", test_passed, test_count);
  
  if (test_passed == test_count) {
    printf("All material properties tests PASSED!\n");
    return 0;
  } else {
    printf("Some material properties tests FAILED!\n");
    return 1;
  }
}