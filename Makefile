SRCDIR = src
BUILDDIR = build

SRCS = src/main.c src/onto.c src/shell.c \
       src/ast.c src/exec.c \
       src/builtin.c \
       build/lex.yy.c build/parse.tab.c

CCFLAGS = -g

build/ontc : $(SRCS)
	$(CC) $(CCFLAGS) -I$(SRCDIR) -I$(BUILDDIR) -o $(BUILDDIR)/ontc $(SRCS)

build/lex.yy.c : src/lex.l build/parse.tab.h
	flex -o $@ $<

build/parse.tab.c build/parse.tab.h : src/parse.y
	bison -o build/parse.tab.c --defines=build/parse.tab.h $<

.PHONY: clean
clean :
	-rm build/*
