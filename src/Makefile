CC = gcc

CFLAGS = -g -Wall -O

INCLUDES = -I /usr/include/python3.8

LIBS = -lrt -lpthread -lpython3.8

SRCS = main.c config_radio.c

OBJS = $(SRCS:.c=.o)

MAIN = radio

.PHONY: depend clean

all:	$(MAIN)
	@echo  radio has been compiled

$(MAIN): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(MAIN) $(OBJS) $(LFLAGS) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<  -o $@

clean:
	$(RM) *.o *~ $(MAIN)

depend: $(SRCS)
	makedepend $(INCLUDES) $^

