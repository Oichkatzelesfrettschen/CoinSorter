#ifndef ENV_H
#define ENV_H
/** \file env.h
 *  \brief Simple physical environment descriptors used by physics demos.
 */

/** \brief Environment parameter bundle. */
typedef struct {
  const char *name;     /**< identifier: earth, moon, mars, orbit */
  double g;             /**< gravitational acceleration (m/s^2) */
  double temperature_K; /**< ambient temperature (Kelvin) */
  double pressure_kPa;  /**< ambient pressure (kPa) */
} Environment;

/** \brief Retrieve an environment by name (defaults to first if NULL). */
const Environment *get_environment(const char *name);
/** \brief Print supported environments to stdout. */
void list_environments(void);

#endif /* ENV_H */
