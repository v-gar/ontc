SRCDIR = src

SRCS = src/main.c src/onto.c src/shell.c \
       # build/lex.yy.c build/parse.tab.c

CCFLAGS = -g

build/ontc : $(SRCS)
	$(CC) $(CCFLAGS) -I$(SRCDIR) -o build/ontc $(SRCS)

build/parser : build/lex.yy.c build/parse.tab.c
	$(CC) $(CCFLAGS) -I$(SRCDIR) -o build/parser $^

build/lex.yy.c : src/lex.l build/parse.tab.h
	flex -o $@ $<

build/parse.tab.c build/parse.tab.h : src/parse.y
	bison -o build/parse.tab.c --defines=build/parse.tab.h $<

.PHONY: parser
parser : build/parser

.PHONY: clean
clean :
	-rm build/*
