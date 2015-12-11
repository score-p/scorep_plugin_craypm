! Copyright (c) 2014 Harvey Richardson
! All rights reserved.
!
! See the LICENSE file elsewhere in this distribution for the
! terms under which this software is made available.

program pm_testf
 use pm_lib
 implicit none
 integer(selected_int_kind(9)),parameter :: i64=selected_int_kind(18)
 integer(kind=i64) :: energy,accel_energy,startup
 integer(kind=i64), allocatable, dimension(:) :: values
 integer power,accel_power,freshness,i,k,nopen,nc,rc
 type(pm_counter) :: mycounters(2)=[ PM_COUNTER_FRESHNESS, PM_COUNTER_ENERGY ]
 type(pm_counter) :: counterworld(3)=[ PM_COUNTER_FRESHNESS, PM_COUNTER_ENERGY, PM_COUNTER_STARTUP ]
 integer(kind=i64) :: myvalues(size(mycounters))
 integer nmc
 character*(20) :: clabel

 call pm_init
! call pm_init(counterworld) ! only open a few

 nmc=size(mycounters)
 startup = pm_get_startup()
 print *,"startup = ",startup
 nc=pm_get_num_opencounters()
 print *,"Number of open counters: ",nc

 print *,'power caps',pm_get_power_cap(), pm_get_accel_power_cap()
 allocate(values(nc))
 do i=1,10
  power = pm_get_power()
  freshness = pm_get_freshness()
  energy = pm_get_energy()
  print *,freshness,' fresh: power ',power,' energy ',energy
  if (nc>4) then
    accel_power = pm_get_accel_power()
    accel_energy = pm_get_accel_energy()    
    print *,' ... accel_power ',accel_power,' accel_energy ',accel_energy
   end if
  nc=pm_get_counters_firstn(3,values,1);
  print *,'first 3 counters:',values(1:3)  
  nc=pm_get_counters(nmc,mycounters,myvalues,1)
  print *,'my counters:',nmc
  do k=1,nmc  
   call pm_get_counter_label(mycounters(k),clabel)
   print *,'  ',trim(clabel),' ',myvalues(k)
  end do
  call sleep(1)
 end do

 call pm_close

end program pm_testf

