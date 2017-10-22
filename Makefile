#****h* bz/Makefile
# NAME
#  Makefile
# FUNCTION
#  Top-level GNU Makefile for bz project (src/bz/Makefile)
#  Requires GNU Make, tested with versions 3.74, 3.79, and 3.80.
# USES
#  robodoc
#  GNU indent
#  Exuberant Ctags
#***** 

MAKEFLAGS       = 
PROJECT_NAME    = bz
PROJECT_VERSION = 0.05
PROJECT_RELEASE = 1
PROJECT         = $(PROJECT_NAME)-$(PROJECT_VERSION)
TARBALL         = $(PROJECT).tar.gz
INCDIRNAME      = include
LIBDIRNAME      = lib
BINDIRNAME      = bin
BAKDIRNAME      = bak
DOCDIRNAME      = doc
TMPDIR          = /tmp
INSTALL         = install -m 755
RM              = rm -f
RMDIR           = rm -fr
MKDIR           = mkdir -p
TCLSH           = tclsh
CONVERT         = convert
ROBODOC         = robodoc
ROBODOCTYPE     = --html
LATEX           = latex
DVIPDF          = dvipdf
JPGIND          = jpgind -E off --style default -s 64 -x 5 -y 5
OCTAVE          = octave --quiet
JPGINCDIR       = $(HOME)/wrk/jpeg-6b
JPGLIBDIR       = $(JPGINCDIR)
MY_CC           = cc
MY_DEBUG_FLAGS  = -Wall -g
MY_OPT_FLAGS    = -Wall -O3
OSTYPE         := $(shell uname)

# the following should be ok for i686/P-Pro/P-II/P-III
# -O9 -funroll-loops -ffast-math -malign-double -mcpu=pentiumpro -march=pentiumpro -fomit-frame-pointer -fno-exceptions


#****v* Makefile/AR
# NAME
#  AR
# DESCRIPTION
#  Archiver definition.
# SEE ALSO
#  RANLIB
# SOURCE

AR              = ar rc

#*****


#****v* Makefile/NICE
# NAME
#  NICE
# DESCRIPTION
#  Makefile definition for running programs with modified scheduling
#  priority. Clear this definition to disable all tinkering with
#  priorities.
# SOURCE

NICE = nice -n 19

#***** 


#****v* Makefile/RANLIB
# NAME
#  RANLIB
# DESCRIPTION
#  Ranlib definition.
# SEE ALSO
#  AR 
# SOURCE

ifeq "$(OSTYPE)" "Darwin"
RANLIB          = ranlib
else
RANLIB          = echo
endif

#***** 


#
# List of all modules in this project. Order is important since there
# are dependencies between different modules.
#
MODULELIST     = tools bzsim helloworld bz2d bz3d bz3d2 bznd
OPT_MODULELIST = eledata
PACKAGELIST    = Makefile $(MODULELIST) $(OPT_MODULELIST)

#
# Variable MODULE is defined only in the modules. This is just
# a handy way to define different targets depending from where the "make" 
# is issued. There are two choices: top-level or module directory.
#
ifndef MODULE
#
# The project top-level definitions and rules.
#
TARGETS=modules
INSTALLPREFIX=./
INSTALLTARGETS=
else
#
# The the module-level definitions and rules.
#
INSTALLPREFIX=../
#
# Rule "install" is required in every module. Each module defines the
# content for this rule.
#
INSTALLTARGETS=install
endif

#
# Define directories. 
#
INCDIR=$(INSTALLPREFIX)$(INCDIRNAME)
LIBDIR=$(INSTALLPREFIX)$(LIBDIRNAME)
BINDIR=$(INSTALLPREFIX)$(BINDIRNAME)
DOCDIR=$(INSTALLPREFIX)$(DOCDIRNAME)
BAKDIR=$(INSTALLPREFIX)$(BAKDIRNAME)

#
# Check if debug flag is set. If you want to compile debug version 
# of either project or module, do: 'make clean; make -e DEBUG=1'
# 
ifeq "$(DEBUG)" "1"
OPTFLAGS   = $(MY_DEBUG_FLAGS)
else
OPTFLAGS   = $(MY_OPT_FLAGS)
endif
CC         = $(MY_CC)
LINKER     = $(CC)

ifeq "$(USEX11)" "1"
# module uses X11 libraries
CFLAGSX11  = -I/usr/openwin/include
LDFLAGSX11 = -L/usr/openwin/lib
else
CFLAGSX11  = 
LDFLAGSX11 = 
endif

ifeq "$(USEGL)" "1"
ifeq "$(OSTYPE)" "darwin"
CFLAGSGL  = -D_MACOSX
LDFLAGSGL = -framework OpenGL -framework GLUT -lobjc 
else
# default to GNU/Linux - ugly but works...
CFLAGSGL  = 
LDFLAGGL = -lglut
endif
CFLAGSGL += -D_USEGL
endif

ifeq "$(USEJPG)" "1"
CFLAGSJPG  = -I$(JPGINCDIR)
LDFLAGSJPG = -L$(JPGLIBDIR)
else
CFLAGSJPG  = 
LDFLAGSJPG = 
endif

ifeq "$(USEBZSIM)" "1"
LDBZLIBS   = -L$(JPGLIBDIR) -lbzsim -ljpeg -lm 
#TODO: add library to binary dependencies
#LINKEDLIBS += $(LIBDIR)/libbzsim.a
else 
LDBZLIBS   =
endif

ifeq "$(USEMATH)" "1"
# module uses math library
LDMLIBS   = -lm 
else 
LDMLIBS   =
endif

CFLAGS     = $(OPTFLAGS) $(CFLAGSX11) $(CFLAGSJPG) $(CFLAGSGL) -I$(INCDIR)
LDLIBS     = $(LDBZLIBS) $(LDMLIBS)
LDFLAGS    = $(LDFLAGSX11) $(LDFLAGSJPG) $(LDFLAGSGL) -L$(LIBDIR)

#****v* Makefile/CTAGS
# NAME
#  CTAGS
# FUNCTION
#  Specify how to find Exuberant Ctags 5.3.1 (or compatible).
# SOURCE

CTAGS       = $(HOME)/bin/ctags

#*****

#****v* Makefile/ROBOHDRS
# NAME
#  ROBOHDRS
# FUNCTION
#  Specify how to use robohdrs. Module-level rule robodocify adds
#  the module info (option -p) into this command.
# SOURCE

ROBOHDRS    = robohdrs -s -x $(CTAGS)

#*****

#****v* Makefile/INDENT
# NAME
#  INDENT
# FUNCTION
#  Source code formatter settings. This will insert or remove 
#  whitespace(s) into C source code to make it more readable.
#  Following should be added to ~/.emacs:
#
#    (global-font-lock-mode 1)
#    (transient-mark-mode 1)
#    (setq-default tab-width 4)
#    (add-hook 'c-mode-hook '(lambda ()
#      (setq c-basic-offset 4)))
#
# SEE ALSO
#  indent 
# SOURCE

INDENT    = indent -kr -i 4 -ts4 -l256 -npsl

#*****

#
# Default rule "all" 
# We want to be able to build a module without installing it.
# Therefore $(INSTALLTARGETS) is not included in this default rule.
# At the top-level, there are no compiling or modules. Therefore
# `make' at the top-level makes actually `make install' for all modules.
#
all: make_dirs $(TARGETS) 

#
# Pattern rule .c -> .o
# No need to define this since GNU `make' has implicit rule for this.
#.c.o:
#	$(CC) $(CFLAGS) -c $<


ifndef MODULE
#
# Top-level rules.
#
MODULELIBDIR = $(LIBDIR)/$(PROJECT)

.PHONY: clean
clean: 
	@for i in $(MODULELIST); do \
	    if [ -d $$i ] ; then \
                (cd $$i && $(MAKE) clean); \
		fi; \
	    done;
	$(RM) ./*~

.PHONY: reallyclean
reallyclean: clean
	@for i in $(LIBDIR) $(INCDIR) $(BINDIR) $(DOCDIR) ; do \
	    if [ -d $$i ] ; then \
		$(RMDIR) $$i; \
		else true; \
		fi; \
	    done;

.PHONY: modules
modules: 
	@for i in $(MODULELIST); do \
	    if [ -d $$i ] ; then \
                (cd $$i && echo "Making module $$i" && $(MAKE) && $(MAKE) install); \
		fi; \
	    done;

#****i* Makefile/snapshot
# NAME
#  snapshot
# FUNCTION
#  Build a source tarball of this project and install it on webserver.
#  On the internet the URL is http://www.iki.fi/petterik/bz/
# SOURCE

WEBHOST=timetelebi
.PHONY: snapshot
snapshot:
	$(MAKE) clean
	$(RMDIR) /tmp/$(PROJECT)
	mkdir -p /tmp/$(PROJECT)/wrk
	cp -R . /tmp/$(PROJECT)/wrk/$(PROJECT)
	find /tmp/$(PROJECT) -type d -name CVS | xargs rm -fr
	find /tmp/$(PROJECT) -type d -name .gone | xargs rm -fr
	find /tmp/$(PROJECT) -type f -name .DS_Store | xargs rm -fr
	find /tmp/$(PROJECT) -type f -name .gdb_history | xargs rm -fr
	(cd /tmp/$(PROJECT); tar cf - wrk) | gzip -c > $(BAKDIR)/$(TARBALL)
	rm -fr /tmp/$(PROJECT)
	rsh $(WEBHOST) mkdir -p public_html/bz
	rcp $(BAKDIR)/$(TARBALL) $(WEBHOST):public_html/bz
	rsh $(WEBHOST) "cd public_html/bz; rm -f bz-latest.tar.gz ; ln -s $(TARBALL) bz-latest.tar.gz"

#*****

.PHONY: package
package: clean bak
	ls -l $(BAKDIR)/$(PROJECT).tar.gz

.PHONY: test_build
test_build: clean
	tar cf - $(PACKAGELIST) | (cd $(TMPDIR) ; tar xf -)
	(cd $(TMPDIR) ; $(MAKE))

#****v* Makefile/doc
# NAME
#  doc
# FUNCTION
#  Generate autodocs from source.
# SOURCE

.PHONY: doc
doc:
	$(RMDIR) $(TMPDIR)/$(PROJECT)/html
	$(RMDIR) $(DOCDIR)/html
	$(MKDIR) $(TMPDIR)/$(PROJECT)/html
	$(ROBODOC) --src ./ --doc $(TMPDIR)/$(PROJECT)/html --html --multidoc --index
	$(ROBODOC) --src ./ --doc $(TMPDIR)/$(PROJECT)/x --html --singledoc --toc
	cp -R $(TMPDIR)/$(PROJECT)/html $(DOCDIR)/html
	$(RMDIR) $(TMPDIR)/$(PROJECT)/html

#*****

# End top-level rules.
else 
#
# Module-level rules.
#
MODULELIBDIR = $(LIBDIR)/$(MODULE)

# Rule for static library and its header file
ifneq "$(LIBFILE)" ""
$(LIBFILE): $(LIBOBJECTS)
	$(RM) $(LIBFILE)
	$(AR) $(LIBFILE) $(LIBOBJECTS)
	$(RANLIB) $(LIBFILE)

$(LIBDIR)/$(LIBFILE): $(LIBFILE)
	$(INSTALL) $(LIBFILE) $(LIBDIR)

$(INCDIR)/$(MODULE).h: $(MODULE).h
	$(INSTALL) $(MODULE).h $(INCDIR)
endif

# Rule for executable binary 
ifneq "$(BINFILE)" ""

$(BINFILE): $(BINOBJECTS) $(LINKEDLIBS)

$(BINDIR)/$(BINFILE): $(BINFILE)
	$(INSTALL) $(BINFILE) $(BINDIR)

endif

ifneq "$(BINFILE2)" ""

$(BINFILE2): $(BINOBJECTS2) $(LINKEDLIBS)

$(BINDIR)/$(BINFILE2): $(BINFILE2)
	$(INSTALL) $(BINFILE2) $(BINDIR)

endif

ifneq "$(BINFILE3)" ""

$(BINFILE3): $(BINOBJECTS3) $(LINKEDLIBS)

$(BINDIR)/$(BINFILE3): $(BINFILE3)
	$(INSTALL) $(BINFILE3) $(BINDIR)

endif

#****** Makefile/indent
# NAME
#  indent
# FUNCTION
#  Module-level rule to carry out uniform C-code formatting in one go.
# SOURCE

.PHONY: indent
indent:
	for i in *.c *.h ; do if [ -f $$i ] ; then $(INDENT) $$i ; fi ; done

#***** 

#****** Makefile/robodocify
# NAME
#  robodocify
# FUNCTION
#  Insert ROBODoc headers.
# SOURCE

.PHONY: robodocify
robodocify:
	for i in *.c *.h ; do if [ -f $$i ] ; then $(ROBOHDRS) -p $(MODULE) $$i ; fi ; done

#*****

#****** Makefile/prj
# NAME
#  prj
# FUNCTION
#  Build the whole project from module level.
# SOURCE

.PHONY: prj
prj:
	(cd .. && $(MAKE))

#*****

# End module level rules.
endif

#****** Makefile/make_dirs
# NAME
#  make_dirs
# FUNCTION
#  Create directory structure for installing this project. Common for
#  top-level and modules.
# SOURCE

.PHONY: make_dirs
make_dirs:
	@for i in $(LIBDIR) $(INCDIR) $(BINDIR) $(DOCDIR) $(MODULELIBDIR); do \
	    if [ ! -d $$i ] ; then \
		$(MKDIR) $$i; \
		else true; \
		fi; \
	    done;

#*****

.PHONY: dist_clean
dist_clean:
	@for i in $(LIBDIR)/$(LIBFILE) \
		  $(INCDIR)/$(MODULE).h $(BINDIR)/$(MODULE); \
	    do \
	    if [ -f $$i ] ; then \
		$(RM) $$i; \
		fi; \
	    done;

