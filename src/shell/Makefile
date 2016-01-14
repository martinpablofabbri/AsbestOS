CFLAGS = -Wall -g

OBJS = mysh.o read.o eval.o command.o

all: mysh

mysh: $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ -lreadline $(LDFLAGS)

clean: 
	rm -f *.o *~ mysh

.PHONY: all clean

