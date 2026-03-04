TARGET = ../bin/sq

SRCS = $(wildcard *.cpp)
OBJS = $(patsubst %.cpp,%.o,$(SRCS))
DEPS = $(patsubst %.cpp,%.d,$(SRCS))

DEPFLAGS = -MMD -MP -MF $*.d
CXXFLAGS += -O2 -fno-exceptions -fno-rtti -Wall -fno-strict-aliasing -I../include $(DEPFLAGS)
LDFLAGS += -L../lib -lsquirrel -lsqstdlib

all: $(TARGET)

$(TARGET): $(OBJS) ../lib/libsquirrel.a ../lib/libsqstdlib.a
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(TARGET) *.o *.d