##########################################################
#
# Toplevel makefile builds all sample applications
# libraries for Embedded Artists' QuickStart Boards.
# (C) 2001-2008 Embedded Artists AB
#
##########################################################

SHELL = bash --posix

all:
ifdef SUBDIRS
	@for dir in $(SUBDIRS) ; do \
	  (cd $$dir && $(MAKE) MAKEFLAGS= all) || exit 1; \
	done
endif

clean:
ifdef SUBDIRS
	@for dir in $(SUBDIRS) ; do \
		(cd $$dir && $(MAKE) MAKEFLAGS= clean) || exit 1; \
	done
endif
