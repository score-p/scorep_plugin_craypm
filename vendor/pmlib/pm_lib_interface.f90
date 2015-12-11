! Copyright (c) 2014 Harvey Richardson
! All rights reserved.
!
! See the LICENSE file elsewhere in this distribution for the
! terms under which this software is made available.

module pm_lib
  implicit none

  public
  private :: i64
  private pm_init_counters

  integer(selected_int_kind(9)),parameter :: i64=selected_int_kind(18)
  
  ! Make this consistent with the pm_lib.h declarations
  enum, bind(c) :: pm_counter
    enumerator :: PM_COUNTER_FRESHNESS
    enumerator :: PM_COUNTER_POWER
    enumerator :: PM_COUNTER_ENERGY
    enumerator :: PM_COUNTER_ACCEL_POWER
    enumerator :: PM_COUNTER_ACCEL_ENERGY
    enumerator :: PM_COUNTER_STARTUP
    enumerator :: PM_NCOUNTERS
    enumerator :: PM_COUNTER_POWER_CAP
    enumerator :: PM_COUNTER_ACCEL_POWER_CAP
  end enum pm_counter

  interface
   subroutine pm_init_counters(n,counters) bind(c,name='pm_init_counters')
    use, intrinsic :: iso_c_binding
    integer(c_int),value :: n
    integer(c_int),intent(in),optional :: counters(*)
   end subroutine pm_init_counters
  end interface

  interface
   subroutine pm_close() bind(c,name='pm_close')
   end subroutine pm_close
  end interface

  interface
   function pm_get_num_opencounters() &
  &                   bind(c,name='pm_get_num_opencounters')
    use, intrinsic :: iso_c_binding
    integer(c_int) :: pm_get_num_opencounters
   end function pm_get_num_opencounters
  end interface

  interface
   function pm_get_freshness() bind(c,name='pm_get_freshness')
    use, intrinsic :: iso_c_binding
    integer(c_int) :: pm_get_freshness
   end function pm_get_freshness
  end interface

  interface
   function pm_get_power() bind(c,name='pm_get_power')
    use, intrinsic :: iso_c_binding
    integer(c_int) :: pm_get_power
   end function pm_get_power
  end interface

  interface
   function pm_get_power_cap() bind(c,name='pm_get_power_cap')
    use, intrinsic :: iso_c_binding
    integer(c_int) :: pm_get_power_cap
   end function pm_get_power_cap
  end interface

  interface
   function pm_get_energy() bind(c,name='pm_get_energy')
    use, intrinsic :: iso_c_binding
    integer(c_long_long) :: pm_get_energy
   end function pm_get_energy
  end interface

  interface
   function pm_get_accel_power() bind(c,name='pm_get_accel_power')
    use, intrinsic :: iso_c_binding
    integer(c_int) :: pm_get_accel_power
   end function pm_get_accel_power
  end interface

  interface
   function pm_get_accel_power_cap() bind(c,name='pm_get_accel_power_cap')
    use, intrinsic :: iso_c_binding
    integer(c_int) :: pm_get_accel_power_cap
   end function pm_get_accel_power_cap
  end interface

  interface
   function pm_get_accel_energy() bind(c,name='pm_get_accel_energy')
    use, intrinsic :: iso_c_binding
    integer(c_long_long) :: pm_get_accel_energy
   end function pm_get_accel_energy
  end interface

  interface
   function pm_get_startup() bind(c,name='pm_get_startup')
    use, intrinsic :: iso_c_binding
    integer(c_long_long) :: pm_get_startup
   end function pm_get_startup
  end interface

  interface
   function pm_get_open_counters() bind(c,name='pm_get_open_counters')
    use, intrinsic :: iso_c_binding
    integer(c_int) :: pm_get_open_counters
   end function pm_get_open_counters
  end interface
     
  interface
   function pm_get_counters_firstn(n,values,atomic) &
  &            bind(c,name='pm_get_counters_firstn')
    use, intrinsic :: iso_c_binding
    integer(c_int) :: pm_get_counters_firstn
    integer(c_int),value :: n
    integer(c_long_long) :: values(*)
    integer(c_int),value :: atomic
   end function pm_get_counters_firstn
  end interface

  interface
   function pm_get_counters(n,counters,values,atomic) &
  &            bind(c,name='pm_get_counters')
    use, intrinsic :: iso_c_binding
    integer(c_int) :: pm_get_counters
    integer(c_int),value :: n
    integer(c_int),       intent(in) :: counters(*)
    integer(c_long_long), intent(out) :: values(*)
    integer(c_int),value :: atomic
   end function pm_get_counters
  end interface

  interface
   function c_pm_get_counter_label(counter) &
  &             bind(c,name='pm_get_counter_label')
    use, intrinsic :: iso_c_binding
    integer(c_int), value :: counter
    type(c_ptr) :: c_pm_get_counter_label
   end function c_pm_get_counter_label
  end interface

  contains
  
   subroutine pm_init(counters)
    use, intrinsic :: iso_c_binding
    implicit none
    type(pm_counter),optional, intent(in) :: counters(:)

    if (present(counters)) then
      call pm_init_counters(size(counters),counters)
     else
      call pm_init_counters(0) ! optional matches null poinrter
     end if

   end subroutine pm_init

   subroutine pm_get_counter_label(counter,label)
    use, intrinsic :: iso_c_binding
    implicit none
    type(pm_counter), intent(in) :: counter
    character*(*), intent(out) :: label
    character(len=1),pointer, dimension(:) :: label_tmp
    integer i,length
 
    length=len(label)
    label=''
    call c_f_pointer(c_pm_get_counter_label(counter),label_tmp,[length])
    do i=1,length
      if (label_tmp(i)==C_NULL_CHAR) exit
      label(i:i)=label_tmp(i)
     end do

   end subroutine pm_get_counter_label

end module pm_lib
