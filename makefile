SHELL = /bin/sh
GCC = /usr/bin/gcc
CFLAGS = -D$(DBUG) -Wall -fmax-errors=5

# Install paths according to GNU make standards
prefix = /usr/local
exec_prefix = $(prefix)
bindir = $(exec_prefix)/bin
includedir = $(prefix)/include
objdir = ~/Code/Obj

all: $(objdir)/libFAutil.a
$(objdir)/libFAutil.a: $(objdir)/fa_sql_generator.o $(objdir)/fa_sql_handler.o
	ar rs $(objdir)/libFAutil.a $(objdir)/fa_*.o

$(objdir)/fa_sql_generator.o: fa_sql_generator.c $(objdir)/fa_sql_generator_key.o
	$(GCC) $(CFLAGS) -c $< -o $@
$(objdir)/fa_sql_generator_key.o: fa_sql_generator_key.c
	$(GCC) $(CFLAGS) -c $^ -o $@
$(objdir)/fa_sql_handler.o: fa_sql_handler.c
	$(GCC) $(CFLAGS) -c $^ -o $@

clean:
	@rm *~

install: $(includedir)/fa_def.h $(includedir)/fa_sql_def.h $(includedir)/fa_sql_lun.h
$(includedir)/fa_def.h: fa_def.h
	sudo cp $^ $@
$(includedir)/fa_sql_def.h: fa_sql_def.h
	sudo cp $^ $@
$(includedir)/fa_sql_lun.h: fa_sql_lun.h
	sudo cp $^ $@

uninstall:
	sudo rm $(includedir)/fa_def.h
	sudo rm $(includedir)/fa_sql_def.h
	sudo rm $(includedir)/fa_sql_lun.h
