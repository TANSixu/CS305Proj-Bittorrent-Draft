OBJFILES = bitflood.o bf_parse.o sha.o spiffy.o
TARGET = bitflood
COMPILE_FLAGS = -g -Wall -Werror
DEBUG_FLAGS = -DDEBUG # -DSPIFFY_ROUTING
CC = gcc

all: $(TARGET)

$(TARGET): $(OBJFILES)
	$(CC) $(COMPILE_FLAGS) $(DEBUG_FLAGS) $(OBJFILES) -o $(TARGET)

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm *~ *.o $(TARGET)
