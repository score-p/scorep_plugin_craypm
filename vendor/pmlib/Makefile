version=1.13

FC=ftn
CC=gcc -std=c99

TAR=pm_lib-v$(version).tar

all: pm_lib.o tests

debug:
	$(MAKE) CFLAGS=-g

pm_lib.o: pm_lib.c
	$(CC) $(CFLAGS) -c pm_lib.c

tests: pm_test pm_testf

pm_test: pm_test.c pm_lib.o pm_lib.h
	$(CC) $(CFLAGS) -o pm_test pm_test.c pm_lib.o

pm_lib_interface.o: pm_lib_interface.f90
	$(FC) -c pm_lib_interface.f90

pm_testf: pm_testf.f90 pm_lib_interface.o pm_lib.o
	$(FC) -o pm_testf pm_testf.f90 pm_lib_interface.o pm_lib.o

clean: 
	$(RM) pm_test pm_testf *.o

tar: LICENSE README USAGE Makefile pm_lib.c pm_lib.h pm_lib_interface.f90 pm_test.c pm_testf.f90
	tar --transform "s:^:pm_lib-$(version)/:" -hcf $(TAR) $^ sys/

