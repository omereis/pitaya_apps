# Versioning system
VERSION ?= 0.00-0000
REVISION ?= devbuild 

CC=$(CROSS_COMPILE)gcc

OBJS = read_signal.o

TARGET = read_signal

CFLAGS  = -g -std=gnu99 -Wall -Werror
CFLAGS += -I../../api/include
CFLAGS += -I/opt/redpitaya/include
CFLAGS += -DVERSION=$(VERSION) -DREVISION=$(REVISION) 
CFLAGS +=  -lm -lpthread 
#CFLAGS += -L /opt/erdpitaya/lib/librp.a
LDFLAGS = -L/opt/redpitaya/lib
LDLIBS = -lm -lpthread -l:librp.so
#CFLAGS +=  -librp

%.o: %.c
	$(CC) -c $(CFLAGS) $(LDFLAGS) $(LDLIBS) $< -o $@ 

TARGET = read_signal

$(TARGET): $(OBJS)
	$(CC) -o $(OBJS) $(CFLAGS) $(LDFLAGS) $(LDLIBS)

clean:
	rm -f $(TARGET) *.o
 
