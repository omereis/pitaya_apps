# Versioning system
VERSION ?= 0.00-0000
REVISION ?= devbuild 

CC=$(CROSS_COMPILE)g++

OBJS = psd.o

TARGET = psd

CFLAGS  = -g -Wall -Werror
CFLAGS += -I../../api/include
CFLAGS += -I/opt/redpitaya/include
CFLAGS += -DVERSION=$(VERSION) -DREVISION=$(REVISION) 
LDFLAGS = -L/opt/redpitaya/lib
LDLAGS +=  -lm -lpthread
LDLIBS = -lm -lpthread -l:librp.so
#CFLAGS +=  -librp

%.o: %.cpp
	$(CC) -c $(CFLAGS) $< -o $@

#TARGET = read_signal
TARGET = psd

$(TARGET): $(OBJS)
	$(CC) -o $(TARGET) $(OBJS) $(LDFLAGS) $(LDLIBS)

clean:
	rm -f $(TARGET) *.o
 
