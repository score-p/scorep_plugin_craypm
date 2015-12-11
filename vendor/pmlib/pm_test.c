/*

 Copyright (c) 2014 Harvey Richardson
 All rights reserved.

 See the LICENSE file elsewhere in this distribution for the
 terms under which this software is made available.

*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "pm_lib.h"

int main(int argc,char **argv){
  int i;
  int power,accel_power,freshness;
  unsigned long long energy,accel_energy,startup;
  int nc;
  int rc;
  pm_counter_value *values;
  pm_counter_e mycounters[2]={ PM_COUNTER_POWER, PM_COUNTER_ENERGY };
  pm_counter_e opencounters[3]={ PM_COUNTER_POWER, PM_COUNTER_ENERGY };
  // or use this if you have GPUs
  //   pm_counter_e mycounters[2]={ PM_COUNTER_POWER, PM_COUNTER_ACCEL_POWER };

  pm_counter_value myvalues[2];
  int num_mycounters;

  //  pm_init_counters(2,opencounters);
  pm_init();
  num_mycounters = sizeof(mycounters)/sizeof(mycounters[0]);

  startup = pm_get_startup();
  printf("startup: %llu\n",startup);
  nc=pm_get_num_opencounters();
  printf("open counters: %d\n",nc);

  values=(pm_counter_value *)malloc(nc*sizeof(pm_counter_value));
  for(i=1;i<20;i++){
    power = pm_get_power();
    freshness = pm_get_freshness();
    energy = pm_get_energy();
    printf("%d fresh: power = %d, energy = %llu",freshness,power,energy);
    if (nc>4) {
      accel_power = pm_get_accel_power();
      accel_energy = pm_get_accel_energy();
      printf(" ,accel_power = %d, accel_energy = %llu",accel_power,accel_energy);
     }
    printf("\n");
    rc=pm_get_counters_firstn(3,values,1);
    printf(" first 3 counters:");
    for(int k=0;k<3;k++)printf(" %llu",values[k]);
    printf("\n");
    rc=pm_get_counters(num_mycounters,mycounters,myvalues,1);
    printf(" my counters (%d):",rc);
    for(int k=0;k<num_mycounters;k++)printf(" %s=%llu",
                               pm_get_counter_label(mycounters[k]),
                               myvalues[k]);
    printf("\n");
    usleep(500000);
   }

  return EXIT_SUCCESS;

}

