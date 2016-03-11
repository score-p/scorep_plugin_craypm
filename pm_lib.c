/*
 * Copyright (c) 2016, Technische Universität Dresden, Germany
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted
 * provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of conditions
 *    and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions
 *    and the following disclaimer in the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse
 *    or promote products derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "pm_lib.h"

/* Files are
 *
 *  version
 *  generation        increments on power cap change
 *  startup           changes on controller restart,  total engery
 *                    only meaningful while this stays constant
 *  freshness         10Hz counter
 *  power_cap      W  Current power cap
 *  power          W  Point in time
 *  energy         J  Accumulated energy
 */

#define MAXFLEN 100
#define MAXLLEN 100
static char *fname[]={
  "freshness",
  "power",
  "energy",
  "startup",
  "accel_power",
  "accel_energy"};

static char syspmcdir[]="/sys/cray/pm_counters";

static FILE *fps[PM_NCOUNTERS];
static int exists[PM_NCOUNTERS];

int pm_init(){
  int k;

  char file[MAXFLEN];

  for(k=0;k<PM_NCOUNTERS;k++){
    strcpy(file,syspmcdir);
    strcat(file,"/");
    strncat(file,fname[k],MAXFLEN);

    if( access( file, F_OK ) == -1 ) {
      exists[k] = 0;
      fps[k] = NULL;
    }else {
      if ( (fps[k]=fopen(file,"r"))==NULL){
        fprintf(stderr,"pm_lib(pm_init): Unable to open %s\n",file);
        exists[k] = 0;
      }else {
        exists[k] = 1;
      }
    }
  }

  k = 0;

  if( exists[PM_FRESHNESS] == 1 && exists[PM_POWER] == 1 && exists[PM_ENERGY] == 1 && exists[PM_STARTUP]) k+=1;
  if( exists[PM_ACCEL_POWER] == 1 && exists[PM_ACCEL_ENERGY] == 1 ) k+=2;

  return k;
}

void pm_close(){
  int k;

  for(k=0;k<PM_NCOUNTERS;k++)
  {
    if ( exists[k] == 1 && fps[k] != NULL ) fclose(fps[k]);
  }
}

static void fgetfirstline(char *line,int size,FILE *fp){
  rewind(fp);
  while(fgets(line,size,fp)!=NULL || errno ==EAGAIN);
}


int pm_get_freshness(){
  int freshness;
  char input_line[MAXLLEN];

  fgetfirstline(input_line,MAXLLEN,fps[PM_FRESHNESS]);
  freshness=atoi(input_line);

  return freshness;

}

int pm_get_power(){
  int power;
  char input_line[MAXLLEN];

  fgetfirstline(input_line,MAXLLEN,fps[PM_POWER]);
  power=atoi(input_line);

  return power;

}

int pm_get_accel_power(){
  int power;
  char input_line[MAXLLEN];

  fgetfirstline(input_line,MAXLLEN,fps[PM_ACCEL_POWER]);
  power=atoi(input_line);

  return power;

}

unsigned long long int pm_get_accel_energy(){
  unsigned long long int energy;
  char input_line[MAXLLEN];

  fgetfirstline(input_line,MAXLLEN,fps[PM_ACCEL_ENERGY]);
  energy=strtoull(input_line,NULL,10);

  return energy;

}

unsigned long long int pm_get_energy(){
  unsigned long long int energy;
  char input_line[MAXLLEN];

  fgetfirstline(input_line,MAXLLEN,fps[PM_ENERGY]);
  energy=strtoull(input_line,NULL,10);

  return energy;

}

unsigned long long int pm_get_startup(){
  unsigned long long int startup;
  char input_line[MAXLLEN];

  fgetfirstline(input_line,MAXLLEN,fps[PM_STARTUP]);
  startup = strtoull(input_line,NULL,10);

  return startup;

}
