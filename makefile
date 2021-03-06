# This makefile has been generated by clice - the command line coding ecosystem
#	more details at www.benningtons.net
# built on 14042017 at 2337

# Shell command variables
SHELL = /bin/sh
GCC = /usr/bin/gcc
CFLAGS= -D$(GXT_DEBUG) -std=gnu11 -Wall -fmax-errors=5

# Install paths according to GNU make standards
prefix = /usr/local
bindir = $(prefix)/bin
includedir = $(prefix)/include
completedir = /etc/bash_completion.d
objdir = $(GXT_CODE_OBJECT)

# This project's executable programs
all:	\
	$(objdir)/libgxtfa.a 

# Tidy-up.
clean:
	-rm *~

# Install project for operational use
install:	\
	$(includedir)/fa_def.h $(includedir)/fa_lun.h $(includedir)/fa_sql_def.h 
$(includedir)/fa_def.h: fa_def.h
	sudo cp $^ $@
$(includedir)/fa_lun.h: fa_lun.h
	sudo cp $^ $@
$(includedir)/fa_sql_def.h: fa_sql_def.h
	sudo cp $^ $@

# Remove project from operational use
uninstall:
	sudo rm $(includedir)/fa_def.h
	sudo rm $(includedir)/fa_lun.h
	sudo rm $(includedir)/fa_sql_def.h

# Functions and their dependencies

$(objdir)/libgxtfa.a: $(objdir)/fa_handler.o $(objdir)/fa_sql_generator.o $(objdir)/fa_sql_generator_key.o \
	 $(objdir)/fa_sql_handler.o 
	ar rs $(objdir)/libgxtfa.a $(objdir)/fa_handler.o $(objdir)/fa_sql_generator.o \
	 $(objdir)/fa_sql_generator_key.o $(objdir)/fa_sql_handler.o
$(objdir)/fa_handler.o: fa_handler.c $(includedir)/fa_def.h $(includedir)/fa_lun.h \
	 $(includedir)/fa_sql_def.h $(objdir)/libgxtfa.a $(includedir)/ut_error.h 
	$(GCC) $(CFLAGS) -c $< -o $@
$(objdir)/fa_sql_generator.o: fa_sql_generator.c $(includedir)/fa_def.h $(includedir)/fa_sql_def.h \
	 $(objdir)/libgxtfa.a $(includedir)/ut_error.h 
	$(GCC) $(CFLAGS) -c $< -o $@
$(objdir)/fa_sql_generator_key.o: fa_sql_generator_key.c $(includedir)/fa_sql_def.h \
	 $(includedir)/ut_error.h 
	$(GCC) $(CFLAGS) -c $< -o $@
$(objdir)/fa_sql_handler.o: fa_sql_handler.c $(includedir)/fa_def.h $(includedir)/fa_lun.h \
	 $(includedir)/fa_sql_def.h $(includedir)/ut_error.h 
	$(GCC) $(CFLAGS) -c $< -o $@
