SRCS = src/main.c src/onto.c src/shell.c

CCFLAGS = -g

ontc : $(SRCS)
	$(CC) $(CCFLAGS) -o ontc $(SRCS)
