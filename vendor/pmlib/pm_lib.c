/*

 Copyright (c) 2014 Harvey Richardson
 All rights reserved.

 See the LICENSE file elsewhere in this distribution for the
 terms under which this software is made available.

*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "pm_lib.h"

/* Files are
 *
 *  version
 *  generation         increments on power cap change
 *  startup            changes on controller restart,  total engery
 *                     only meaningful while this stays constant
 *  freshness          10Hz counter
 *  energy          J  Accumulated energy
 *  power           W  Point in time
 *  power_cap       W  Current power cap
 *  accel_power     W  Accelerator power
 "  accel_power_cap W
 *  accel_energy    J  Accelerator energy
 */

/* This is the number of times we will retry to get atomic counter values */
#define PM_MAX_RETRY 10
static long long retries_total,retries_last;

#define MAXFLEN 100
#define MAXLLEN 100
static char *fname[]={
  "freshness",
  "power",
  "energy",
  "accel_power",
  "accel_energy",
  "startup",
  "power_cap",
  "accel_power_cap"};


static char syspmcdir[]="/sys/cray/pm_counters";
//static char syspmcdir[]="/users/harveyr/Power/sys/cray/pm_counters";

static int nopen=0;  // Number of counter files open
static FILE *fps[PM_NCOUNTERS];

void pm_init_counters(int n,pm_counter_e *counters){
  int i,k;
  int nc;
  int opened_freshness=0;
  char file[MAXFLEN];

  if (n==0) {
    nc=PM_NCOUNTERS;
   } else {
    nc=n;
   }
 
  for(i=0;i<nc;i++){

    if (n==0) {
      k=i;
     } else {
      k=counters[i];
     }
    if (k==PM_COUNTER_FRESHNESS) opened_freshness=1;
    strcpy(file,syspmcdir);
    strcat(file,"/");
    strncat(file,fname[k],MAXFLEN);

    if ( (fps[k]=fopen(file,"r"))!=NULL){
      //             fprintf(stderr,"pm_lib(pm_init): Opened %s\n",file);
      nopen++;
     } else {
      if (! (k==PM_COUNTER_ACCEL_POWER || 
             k==PM_COUNTER_ACCEL_ENERGY ||
             k==PM_COUNTER_ACCEL_POWER_CAP) ) {
       fprintf(stderr,"pm_lib(pm_init): Failed to open file %s\n",file);
       }
     }
  }

  if (! opened_freshness){
    k=PM_COUNTER_FRESHNESS;
    strcpy(file,syspmcdir);
    strcat(file,"/");
    strncat(file,fname[k],MAXFLEN);
    if ( (fps[k]=fopen(file,"r"))!=NULL){
      nopen++;
      //      fprintf(stderr,"pm_lib(pm_init): had to open %s\n",file);
     } else {
       fprintf(stderr,"pm_lib(pm_init): Failed to open file %s\n",file);
     }

  }

}

void pm_init(){
  pm_init_counters(0,NULL);
}

void pm_close(){

  for(int k=0;k<PM_NCOUNTERS;k++) if (fps[k]!=NULL) fclose(fps[k]);

}

// Return the number of coutner files that are currently open
int pm_get_num_opencounters(){
  return nopen;
}

// Return first line from file into line
// Do not alter line if file is not already open
static void fgetfirstline(char *line,int size,FILE *fp){
  
  if (fp==NULL) return;

  rewind(fp);
  while(fgets(line,size,fp)!=NULL || errno ==EAGAIN);
}

// return first n counters
int pm_get_counters_firstn(int n,pm_counter_value *values,int atomic){
  char input_line[MAXLLEN];
  int nr,nloop;
  int freshness1,freshness2;
 
  fgetfirstline(input_line,MAXLLEN,fps[PM_COUNTER_FRESHNESS]);
  freshness1=atoi(input_line);
  //  printf("freshness l1 = %d\n",freshness1);
  values[PM_COUNTER_FRESHNESS]=freshness1;

  nloop=0;
  do {
    if (nloop>0) freshness1=freshness2;
    nr=1;
    for(int k=1;k<n;k++){
      if (fps[k]!=NULL){
        fgetfirstline(input_line,MAXLLEN,fps[k]);
        values[k]=strtoull(input_line,NULL,10);
        nr++;   
       }  else {
        values[k]=0;
       }
     }
    if (atomic){
      fgetfirstline(input_line,MAXLLEN,fps[PM_COUNTER_FRESHNESS]);
      freshness2=atoi(input_line);
      //      printf("freshness l2 = %d\n",freshness2);
      }
  } while (atomic && nloop++<PM_MAX_RETRY && freshness1!=freshness2 );

  if (atomic) {
    retries_last=(nloop-1);
    retries_total+=(nloop-1);
  }

  if (atomic && freshness1 !=freshness2 ){
    //    printf("returning 0\n");
    return 0;
   } else {
    return nr;
   }

}

char* pm_get_counter_label(pm_counter_e counter){

  return fname[counter];

}


// Make sure we return counters bracketed by a read of the SAME
// freshness
// Optimize the case where one of the counters to be returned
// is the freshness so we don't read too often
static int pm_get_counters_atomic(int n,pm_counter_e *counters,
                                  pm_counter_value *values){
  char input_line[MAXLLEN];
  int nr,nloop;
  int freshness1,freshness2;
 
  fgetfirstline(input_line,MAXLLEN,fps[PM_COUNTER_FRESHNESS]);
  freshness1=atoi(input_line);
  //  printf("freshness l1 = %d\n",freshness1);

  nloop=0;
  do {
    if (nloop>0) freshness1=freshness2;
    nr=0;
    for(int k=0;k<n;k++){
      // Note that we can't skip the freshness read if we are last
      // at the same time as picking it up after the loop
      if (counters[k]==PM_COUNTER_FRESHNESS && k<n-1){
        values[k]=freshness1;
	//        printf("taking earlier freshness for value %d\n",k);
       } else {
        if (fps[counters[k]]!=NULL) {
          fgetfirstline(input_line,MAXLLEN,fps[counters[k]]);
          values[k]=strtoull(input_line,NULL,10);
 	 } else {
          values[k]=0;
          nr--;
	 }
        }
       nr++;   
    }
    if (counters[n-1]==PM_COUNTER_FRESHNESS){
      freshness2=values[n-1];
      //      printf("freshness l2 (from loop) = %d\n",freshness2);
     } else {
      fgetfirstline(input_line,MAXLLEN,fps[PM_COUNTER_FRESHNESS]);
      freshness2=atoi(input_line);
      //      printf("freshness l2 (from file after loop) = %d\n",freshness2);
     }
     
  } while (nloop++<PM_MAX_RETRY && freshness1!=freshness2 );

  retries_last=(nloop-1);
  retries_total+=(nloop-1);

  if (freshness1 !=freshness2 ){
    printf("returning 0\n");
    return 0;
   } else {
    return nr;
   }

}


int pm_get_counters(int n,pm_counter_e *counters,
                    pm_counter_value *values,int atomic){
  char input_line[MAXLLEN];
  int nr;

  if (atomic && n>1) return pm_get_counters_atomic(n,counters,values);

  // Only get here if this was not an atomic request
  nr=0;
  for(int k=0;k<n;k++){
   fgetfirstline(input_line,MAXLLEN,fps[counters[k]]);
   values[k]=strtoull(input_line,NULL,10);
   nr++;   
  }
  return nr;

}


int pm_get_freshness(){
  int freshness;
  char input_line[MAXLLEN];

  fgetfirstline(input_line,MAXLLEN,fps[PM_COUNTER_FRESHNESS]);
  freshness=atoi(input_line);
  
  return freshness;

}

int pm_get_power(){
  int power;
  char input_line[MAXLLEN];

  fgetfirstline(input_line,MAXLLEN,fps[PM_COUNTER_POWER]);
  power=atoi(input_line);
  
  return power;

}

int pm_get_accel_power(){
  int accel_power;
  char input_line[MAXLLEN];

  fgetfirstline(input_line,MAXLLEN,fps[PM_COUNTER_ACCEL_POWER]);
  accel_power=atoi(input_line);
  
  return accel_power;

}

int pm_get_power_cap(){
  int power_cap;
  char input_line[MAXLLEN];

  fgetfirstline(input_line,MAXLLEN,fps[PM_COUNTER_POWER_CAP]);
  power_cap=atoi(input_line);
  
  return power_cap;

}

int pm_get_accel_power_cap(){
  int accel_power_cap;
  char input_line[MAXLLEN];

  fgetfirstline(input_line,MAXLLEN,fps[PM_COUNTER_ACCEL_POWER_CAP]);
  accel_power_cap=atoi(input_line);
  
  return accel_power_cap;

}

unsigned long long int pm_get_energy(){
  unsigned long long int energy;
  char input_line[MAXLLEN];

  fgetfirstline(input_line,MAXLLEN,fps[PM_COUNTER_ENERGY]);
  energy=strtoull(input_line,NULL,10);
  
  return energy;

}


unsigned long long int pm_get_accel_energy(){
  unsigned long long int accel_energy;
  char input_line[MAXLLEN];

  fgetfirstline(input_line,MAXLLEN,fps[PM_COUNTER_ACCEL_ENERGY]);
  accel_energy=strtoull(input_line,NULL,10);
  
  return accel_energy;

}

unsigned long long int pm_get_startup(){
  unsigned long long int startup;
  char input_line[MAXLLEN];

  fgetfirstline(input_line,MAXLLEN,fps[PM_COUNTER_STARTUP]);
  startup = strtoull(input_line,NULL,10);
  
  return startup;

}
