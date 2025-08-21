#ifndef ENV_H
#define ENV_H

/* Physical environment parameters for coin handling. */
typedef struct {
  const char *name;     /* identifier: earth, moon, mars, orbit */
  double g;             /* gravitational acceleration m/s^2 */
  double temperature_K; /* ambient temperature (optional) */
  double pressure_kPa;  /* ambient pressure (kPa) */
} Environment;

const Environment *get_environment(const char *name);
void list_environments(void);

#endif /* ENV_H */
