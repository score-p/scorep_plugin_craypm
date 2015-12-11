/*
 Copyright (c) 2014 Harvey Richardson
 All rights reserved.

 See the LICENSE file elsewhere in this distribution for the
 terms under which this software is made available.

*/

#ifndef PM_LIB_H
#define PM_LIB_H

// Counters supported
typedef enum pm_counters
  { PM_COUNTER_FRESHNESS,  /* The freshness counter MUST be first in the list */
    PM_COUNTER_POWER,
    PM_COUNTER_ENERGY,
    PM_COUNTER_ACCEL_POWER,
    PM_COUNTER_ACCEL_ENERGY,
    PM_COUNTER_STARTUP,
    PM_COUNTER_POWER_CAP,
    PM_COUNTER_ACCEL_POWER_CAP,
    PM_NCOUNTERS
  } pm_counter_e ;

typedef unsigned long long pm_counter_value;

extern void pm_init(void);
extern void pm_init_counters(int n,pm_counter_e *counters);
extern void pm_close(void);
extern int pm_get_num_opencounters();
extern char* pm_get_counter_label(pm_counter_e counter);

// Single counter functions
extern int pm_get_freshness(void);
extern unsigned long long pm_get_startup(void);
extern int pm_get_power(void);
extern unsigned long long pm_get_energy(void);
extern int pm_get_accel_power(void);
extern unsigned long long pm_get_accel_energy(void);
extern int pm_get_accel_power_cap(void);

// Functions to return multiple coutners at once
// Set atomic to 1 to get atomic read (0 otherwise)
extern int pm_get_counters_firstn(int n,pm_counter_value *v,int atomic);
extern int pm_get_counters(int n,pm_counter_e *counters,pm_counter_value *v,int atomic);

#endif /* header guard */
