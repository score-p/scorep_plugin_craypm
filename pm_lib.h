/*
   libpm_plugin.so,
   a library to monitor energy and power counter on Cray machines.
   Copyright (C) 2014 TU Dresden, ZIH
   Copyright (C) 2014 Cray Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, v2, as
   published by the Free Software Foundation

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

extern int pm_init(void);
extern void pm_close(void);
extern int pm_get_power(void);
extern int pm_get_freshness(void);
extern unsigned long long pm_get_energy(void);
extern unsigned long long pm_get_startup(void);
extern int pm_get_accel_power(void);
extern unsigned long long  pm_get_accel_energy(void);

typedef enum pm_counters
  { PM_FRESHNESS,
    PM_POWER,
    PM_ENERGY,
    PM_STARTUP,
    PM_ACCEL_POWER,
    PM_ACCEL_ENERGY,
    PM_NCOUNTERS
  } pm_counters_e ;
