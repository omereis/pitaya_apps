# Versioning system
VERSION ?= 0.00-0000
REVISION ?= devbuild 

CPP=$(CROSS_COMPILE)g++

OBJS = psd.o
SRCS = $(subst .o,.c, $(OBJS)))

TARGET = psd

CFLAGS  = -g -Wall -Werror
CFLAGS +=  -std=c++0x
CFLAGS += -I../../api/include
CFLAGS += -I/opt/redpitaya/include
CFLAGS += -DVERSION=$(VERSION) -DREVISION=$(REVISION) 
LDFLAGS = -L/opt/redpitaya/lib
LDLAGS +=  -lm -lpthread
LDLIBS = -lm -lpthread -l:librp.so
#CFLAGS +=  -librp

%.o: %.cpp
#	$(CC) -c $(CFLAGS) $< -o $@
	$(CPP) -c -o  $@  $< $(CFLAGS)

#TARGET = read_signal
#TARGET = psd

$(TARGET): $(OBJS)
	$(CPP) -o $(TARGET) $(OBJS) $(LDFLAGS) $(LDLIBS)

clean:
	rm -f $(TARGET) *.o
 
