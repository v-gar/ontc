BISON = bison
FLEX = flex
MKDIR = mkdir

SRCDIR = src
LIBDIR = lib
INCDIR = include
BUILDDIR = build

OXPLLIBDIR = $(LIBDIR)/oxpl
ONTGLIBDIR = $(LIBDIR)/ontg

OXPLINCDIR = $(INCDIR)/oxpl
ONTGINCDIR = $(INCDIR)/ontg

OXPLBUILDDIR = $(BUILDDIR)/oxpl
ONTGBUILDDIR = $(BUILDDIR)/ontg

SRCS = src/main.c src/shell.c src/exec.c

CCFLAGS = -g

OXPLLIBOBJS = $(OXPLBUILDDIR)/ast.o $(OXPLBUILDDIR)/builtin.o\
	      $(OXPLBUILDDIR)/lex.yy.o $(OXPLBUILDDIR)/parse.tab.o
ONTGLIBOBJS = $(ONTGBUILDDIR)/onto.o

OXPLBISONH = $(OXPLBUILDDIR)/parse.tab.h
OXPLBISONP = $(OXPLBUILDDIR)/parse.tab.c

# BINS

build/ontc : $(SRCS) build/libontg.a build/liboxpl.a
	$(CC) $(CCFLAGS) -I$(SRCDIR) -I$(BUILDDIR)/oxpl -I$(OXPLINCDIR) \
		-I$(ONTGINCDIR) -Lbuild -lontg -loxpl -o $(BUILDDIR)/ontc $^

# OXPL

$(OXPLBUILDDIR)/lex.yy.c : lib/oxpl/lex.l $(OXPLBISONH) $(OXPLBUILDDIR)
	$(FLEX) -o $@ $<

$(OXPLBISONP) $(OXPLBISONH) : lib/oxpl/parse.y $(OXPLBUILDDIR)
	$(BISON) -o $@ --defines=$(OXPLBUILDDIR)/parse.tab.h $<

$(OXPLBUILDDIR)/%.o : lib/oxpl/%.c $(OXPLBISONP) $(OXPLBISONH) $(OXPLBUILDDIR)
	$(CC) $(CCFLAGS) -I$(OXPLINCDIR) -I$(OXPLLIBDIR) -c -o $@ $<

$(OXPLBUILDDIR)/%.yy.o : $(OXPLBUILDDIR)/%.yy.c  $(OXPLBUILDDIR)
	$(CC) $(CCFLAGS) -I$(OXPLINCDIR) -I$(OXPLLIBDIR) -c -o $@ $<

$(OXPLBUILDDIR)/%.tab.o : $(OXPLBUILDDIR)/%.tab.c  $(OXPLBUILDDIR)
	$(CC) $(CCFLAGS) -I$(OXPLINCDIR) -I$(OXPLLIBDIR) -c -o $@ $<

# ONTG

$(ONTGBUILDDIR)/%.o : $(ONTGLIBDIR)/%.c $(ONTGBUILDDIR)
	$(CC) $(CCFLAGS) -I$(ONTGINCDIR) -I$(ONTGLIBDIR) -c -o $@ $<

# LIBS

build/liboxpl.a : $(OXPLLIBOBJS) | $(OXPLBUILDDIR)
	$(AR) rcs $@ $^

build/libontg.a : $(ONTGLIBOBJS) | $(ONTGBUILDDIR)
	$(AR) rcs $@ $^

# DIRS
$(OXPLBUILDDIR) $(ONTGBUILDDIR) :
	$(MKDIR) -p $@

.PHONY: clean
clean :
	-rm -r build/*
