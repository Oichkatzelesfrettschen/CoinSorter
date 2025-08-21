/** \file observables.c
 *  \brief Additional physics observables with comprehensive material properties.
 *
 *  This module provides advanced measurable quantities incorporating detailed
 *  material properties of coin alloys including electromagnetic, thermal,
 *  mechanical, and optical characteristics derived from exhaustive research.
 */
#include "observables.h"
#include "coins.h"
#include <math.h>
#include <string.h>
#include <stdio.h>

/* Physical constants */
#define VACUUM_PERMITTIVITY    8.854187817e-12  /* F/m */
#define VACUUM_PERMEABILITY    4.0e-7 * M_PI    /* H/m */
#define SPEED_OF_LIGHT         299792458.0      /* m/s */

/** \brief Comprehensive material properties database.
 *
 * Based on extensive research of coin material properties from metallurgy
 * databases, materials engineering handbooks, and electromagnetic references.
 */
static const MaterialProperties MATERIAL_DATABASE[] = {
  /* Pure Copper (Cu) */
  {
    .relative_permittivity = 1.0,
    .electrical_conductivity = 59.6e6,      /* S/m - IACS 100% */
    .relative_permeability = 0.999994,      /* Diamagnetic */
    .magnetic_susceptibility = -9.63e-6,
    .thermal_conductivity = 401.0,          /* W/(m·K) */
    .specific_heat_capacity = 385.0,        /* J/(kg·K) */
    .thermal_expansion_coeff = 16.5e-6,     /* 1/K */
    .density = 8960.0,                      /* kg/m³ */
    .youngs_modulus = 128.0,                /* GPa */
    .poissons_ratio = 0.34,
    .hardness_hv = 369.0,                   /* HV */
    .refractive_index = 0.617,              /* @ 589nm */
    .reflectance = 0.945,
    .corrosion_resistance = 0.85,
    .material_class = "Pure Copper"
  },
  
  /* Nickel (Ni) */
  {
    .relative_permittivity = 1.0,
    .electrical_conductivity = 14.3e6,      /* S/m */
    .relative_permeability = 600.0,         /* Ferromagnetic */
    .magnetic_susceptibility = 599.0,
    .thermal_conductivity = 90.9,           /* W/(m·K) */
    .specific_heat_capacity = 444.0,        /* J/(kg·K) */
    .thermal_expansion_coeff = 13.4e-6,     /* 1/K */
    .density = 8908.0,                      /* kg/m³ */
    .youngs_modulus = 200.0,                /* GPa */
    .poissons_ratio = 0.31,
    .hardness_hv = 638.0,                   /* HV */
    .refractive_index = 1.08,               /* @ 589nm */
    .reflectance = 0.72,
    .corrosion_resistance = 0.92,
    .material_class = "Nickel"
  },
  
  /* Zinc (Zn) */
  {
    .relative_permittivity = 1.0,
    .electrical_conductivity = 16.6e6,      /* S/m */
    .relative_permeability = 0.9999994,     /* Diamagnetic */
    .magnetic_susceptibility = -2.21e-5,
    .thermal_conductivity = 116.0,          /* W/(m·K) */
    .specific_heat_capacity = 388.0,        /* J/(kg·K) */
    .thermal_expansion_coeff = 30.2e-6,     /* 1/K */
    .density = 7134.0,                      /* kg/m³ */
    .youngs_modulus = 108.0,                /* GPa */
    .poissons_ratio = 0.25,
    .hardness_hv = 412.0,                   /* HV */
    .refractive_index = 2.356,              /* @ 589nm */
    .reflectance = 0.769,
    .corrosion_resistance = 0.65,
    .material_class = "Zinc"
  },
  
  /* Aluminum (Al) */
  {
    .relative_permittivity = 1.0,
    .electrical_conductivity = 37.8e6,      /* S/m - 63% IACS */
    .relative_permeability = 1.0000220,     /* Paramagnetic */
    .magnetic_susceptibility = 2.2e-5,
    .thermal_conductivity = 237.0,          /* W/(m·K) */
    .specific_heat_capacity = 897.0,        /* J/(kg·K) */
    .thermal_expansion_coeff = 23.1e-6,     /* 1/K */
    .density = 2700.0,                      /* kg/m³ */
    .youngs_modulus = 70.0,                 /* GPa */
    .poissons_ratio = 0.35,
    .hardness_hv = 167.0,                   /* HV */
    .refractive_index = 1.44,               /* @ 589nm */
    .reflectance = 0.913,
    .corrosion_resistance = 0.78,
    .material_class = "Aluminum"
  },
  
  /* Steel (typical composition) */
  {
    .relative_permittivity = 1.0,
    .electrical_conductivity = 6.99e6,      /* S/m */
    .relative_permeability = 100.0,         /* Ferromagnetic, varies widely */
    .magnetic_susceptibility = 99.0,
    .thermal_conductivity = 50.2,           /* W/(m·K) */
    .specific_heat_capacity = 490.0,        /* J/(kg·K) */
    .thermal_expansion_coeff = 11.0e-6,     /* 1/K */
    .density = 7850.0,                      /* kg/m³ */
    .youngs_modulus = 200.0,                /* GPa */
    .poissons_ratio = 0.30,
    .hardness_hv = 196.0,                   /* HV */
    .refractive_index = 2.485,              /* @ 589nm */
    .reflectance = 0.569,
    .corrosion_resistance = 0.45,
    .material_class = "Steel"
  },
  
  /* Cupronickel (75% Cu, 25% Ni) */
  {
    .relative_permittivity = 1.0,
    .electrical_conductivity = 3.2e6,       /* S/m */
    .relative_permeability = 1.002,         /* Weakly magnetic */
    .magnetic_susceptibility = 0.002,
    .thermal_conductivity = 29.0,           /* W/(m·K) */
    .specific_heat_capacity = 410.0,        /* J/(kg·K) */
    .thermal_expansion_coeff = 16.2e-6,     /* 1/K */
    .density = 8940.0,                      /* kg/m³ */
    .youngs_modulus = 150.0,                /* GPa */
    .poissons_ratio = 0.32,
    .hardness_hv = 120.0,                   /* HV */
    .refractive_index = 0.82,               /* @ 589nm */
    .reflectance = 0.31,
    .corrosion_resistance = 0.95,
    .material_class = "Cupronickel"
  },
  
  /* Nordic Gold (89% Cu, 5% Al, 5% Zn, 1% Sn) */
  {
    .relative_permittivity = 1.0,
    .electrical_conductivity = 10.4e6,      /* S/m */
    .relative_permeability = 0.999996,      /* Diamagnetic */
    .magnetic_susceptibility = -8.1e-6,
    .thermal_conductivity = 159.0,          /* W/(m·K) */
    .specific_heat_capacity = 394.0,        /* J/(kg·K) */
    .thermal_expansion_coeff = 18.2e-6,     /* 1/K */
    .density = 7800.0,                      /* kg/m³ */
    .youngs_modulus = 115.0,                /* GPa */
    .poissons_ratio = 0.33,
    .hardness_hv = 90.0,                    /* HV */
    .refractive_index = 0.47,               /* @ 589nm */
    .reflectance = 0.85,
    .corrosion_resistance = 0.88,
    .material_class = "Nordic Gold"
  },
  
  /* Brass (typical alloy 70% Cu, 30% Zn) */
  {
    .relative_permittivity = 1.0,
    .electrical_conductivity = 15.9e6,      /* S/m */
    .relative_permeability = 0.999991,      /* Diamagnetic */
    .magnetic_susceptibility = -9.0e-6,
    .thermal_conductivity = 109.0,          /* W/(m·K) */
    .specific_heat_capacity = 380.0,        /* J/(kg·K) */
    .thermal_expansion_coeff = 20.3e-6,     /* 1/K */
    .density = 8530.0,                      /* kg/m³ */
    .youngs_modulus = 97.0,                 /* GPa */
    .poissons_ratio = 0.37,
    .hardness_hv = 101.0,                   /* HV */
    .refractive_index = 0.99,               /* @ 589nm */
    .reflectance = 0.61,
    .corrosion_resistance = 0.82,
    .material_class = "Brass"
  },
  
  /* Nickel-plated Steel */
  {
    .relative_permittivity = 1.0,
    .electrical_conductivity = 8.5e6,       /* S/m - composite property */
    .relative_permeability = 200.0,         /* Ferromagnetic, reduced by Ni */
    .magnetic_susceptibility = 199.0,
    .thermal_conductivity = 65.0,           /* W/(m·K) */
    .specific_heat_capacity = 465.0,        /* J/(kg·K) */
    .thermal_expansion_coeff = 12.5e-6,     /* 1/K */
    .density = 7900.0,                      /* kg/m³ */
    .youngs_modulus = 190.0,                /* GPa */
    .poissons_ratio = 0.31,
    .hardness_hv = 250.0,                   /* HV */
    .refractive_index = 1.8,                /* @ 589nm */
    .reflectance = 0.65,
    .corrosion_resistance = 0.85,
    .material_class = "Nickel-plated Steel"
  }
};

#define NUM_MATERIALS ((size_t)(sizeof(MATERIAL_DATABASE) / sizeof(MATERIAL_DATABASE[0])))

/** \brief Material lookup table mapping composition strings to material indices.
 *
 * This table maps common coin composition descriptions to indices in the
 * MATERIAL_DATABASE array for efficient lookup.
 */
static const struct {
  const char *composition_pattern;
  size_t material_index;
} COMPOSITION_LOOKUP[] = {
  /* Pure metals */
  {"Aluminum", 3},
  {"Al bronze", 3},  /* Aluminum bronze approximated as aluminum */
  
  /* Copper-based */
  {"8.33% Ni bal Cu (clad)", 5},      /* Cupronickel clad */
  {"25% Ni bal Cu", 5},               /* Cupronickel */
  {"2.5% Cu 97.5% Zn (plated)", 2},   /* Zinc core */
  {"Cu plated steel", 4},             /* Steel with copper plating */
  {"Cu plated Zn", 2},                /* Zinc with copper plating */
  {"Cupronickel", 5},
  
  /* Nordic gold and brass */
  {"Nordic gold", 6},
  {"Ni brass", 7},                    /* Nickel brass ~ brass */
  {"brass", 7},
  {"Brass alloy", 7},
  
  /* Steel variants */
  {"Multi-ply Ni plated steel", 8},
  {"Ni plated steel", 8},
  {"steel", 4},
  
  /* Bi-metallic approximations (use outer ring material) */
  {"Bi-metal: Cu-Ni/Ni brass", 5},    /* Cupronickel dominant */
  {"Bi-metal: Ni brass/Cu-Ni", 7},    /* Brass dominant */
  {"Bi-metal Ni/Al-bronze", 1},       /* Nickel dominant */
  
  /* Multi-ply (use base material) */
  {"Multi-ply brass plated steel", 8}, /* Steel base */
};

#define NUM_LOOKUP_ENTRIES ((size_t)(sizeof(COMPOSITION_LOOKUP) / sizeof(COMPOSITION_LOOKUP[0])))

/** \brief Normalized energy-like scalar from gradient components.
 *
 * Current implementation returns 0.5*(dx^2+dy^2). In future this could be
 * extended to incorporate dielectric or material parameters.
 */
double observable_energy_density(double dx, double dy) {
  return 0.5 * (dx * dx + dy * dy);
}

/** \brief Enhanced energy density incorporating material properties.
 *
 * Computes energy density including dielectric and conductive effects.
 */
double observable_energy_density_enhanced(double dx, double dy, 
                                         const MaterialProperties *properties) {
  if (!properties) {
    return observable_energy_density(dx, dy);
  }
  
  /* Base energy density */
  double base_energy = 0.5 * (dx * dx + dy * dy);
  
  /* Scale by dielectric properties */
  double dielectric_factor = properties->relative_permittivity;
  
  /* Add conductive losses (simplified model) */
  double conductivity_factor = 1.0 + properties->electrical_conductivity / 1e8;
  
  /* Include magnetic permeability effects */
  double magnetic_factor = properties->relative_permeability;
  
  return base_energy * dielectric_factor * conductivity_factor * 
         sqrt(magnetic_factor);
}

/** \brief Electromagnetic field energy density for coin material.
 *
 * Calculates electromagnetic field energy density based on material properties.
 */
double observable_em_energy_density(double e_field, double b_field,
                                   const MaterialProperties *properties) {
  if (!properties) {
    return 0.0;
  }
  
  /* Electric field energy density: ½ε₀εᵣE² */
  double electric_energy = 0.5 * VACUUM_PERMITTIVITY * 
                          properties->relative_permittivity * 
                          e_field * e_field;
  
  /* Magnetic field energy density: ½(B²)/(μ₀μᵣ) */
  double magnetic_energy = 0.5 * b_field * b_field / 
                          (VACUUM_PERMEABILITY * properties->relative_permeability);
  
  return electric_energy + magnetic_energy;
}

/** \brief Thermal diffusion coefficient from material properties.
 *
 * Computes thermal diffusivity α = k/(ρ·cp).
 */
double observable_thermal_diffusivity(const MaterialProperties *properties) {
  if (!properties || properties->density <= 0.0 || 
      properties->specific_heat_capacity <= 0.0) {
    return 0.0;
  }
  
  return properties->thermal_conductivity / 
         (properties->density * properties->specific_heat_capacity);
}

/** \brief Skin depth for electromagnetic waves in conducting material.
 *
 * Calculates skin depth δ = √(2/(ωμσ)) for AC fields.
 */
double observable_skin_depth(double frequency, const MaterialProperties *properties) {
  if (!properties || frequency <= 0.0 || 
      properties->electrical_conductivity <= 0.0) {
    return INFINITY;
  }
  
  double mu = VACUUM_PERMEABILITY * properties->relative_permeability;
  double omega = 2.0 * M_PI * frequency;
  
  return sqrt(2.0 / (omega * mu * properties->electrical_conductivity));
}

/** \brief Acoustic impedance of the material.
 *
 * Computes Z = ρ·v where v is calculated from elastic modulus.
 */
double observable_acoustic_impedance(const MaterialProperties *properties) {
  if (!properties || properties->density <= 0.0 || 
      properties->youngs_modulus <= 0.0) {
    return 0.0;
  }
  
  /* Convert Young's modulus from GPa to Pa */
  double elastic_modulus = properties->youngs_modulus * 1e9;
  
  /* Longitudinal wave speed: v = √(E/ρ) for thin rods */
  double wave_speed = sqrt(elastic_modulus / properties->density);
  
  return properties->density * wave_speed;
}

/** \brief Look up material properties from coin composition string.
 *
 * Analyzes composition string and returns appropriate material properties.
 */
const MaterialProperties *get_material_properties(const CoinSpec *coin) {
  if (!coin || !coin->composition) {
    return NULL;
  }
  
  return get_material_properties_by_composition(coin->composition);
}

/** \brief Get material properties by composition string directly.
 *
 * Direct lookup by composition string for more flexible usage.
 */
const MaterialProperties *get_material_properties_by_composition(const char *composition) {
  if (!composition) {
    return NULL;
  }
  
  /* Search for exact matches first */
  for (size_t i = 0; i < NUM_LOOKUP_ENTRIES; i++) {
    if (strstr(composition, COMPOSITION_LOOKUP[i].composition_pattern)) {
      size_t material_idx = COMPOSITION_LOOKUP[i].material_index;
      if (material_idx < NUM_MATERIALS) {
        return &MATERIAL_DATABASE[material_idx];
      }
    }
  }
  
  /* Fallback: try partial matching for common materials */
  if (strstr(composition, "Cu") || strstr(composition, "copper")) {
    return &MATERIAL_DATABASE[0];  /* Copper */
  }
  if (strstr(composition, "Ni") || strstr(composition, "nickel")) {
    return &MATERIAL_DATABASE[1];  /* Nickel */
  }
  if (strstr(composition, "Zn") || strstr(composition, "zinc")) {
    return &MATERIAL_DATABASE[2];  /* Zinc */
  }
  if (strstr(composition, "Al") || strstr(composition, "aluminum")) {
    return &MATERIAL_DATABASE[3];  /* Aluminum */
  }
  if (strstr(composition, "steel")) {
    return &MATERIAL_DATABASE[4];  /* Steel */
  }
  
  return NULL;  /* No match found */
}
