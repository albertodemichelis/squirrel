TARGET = ../lib/libsquirrel.a

SRCS = $(wildcard *.cpp)
OBJS = $(patsubst %.cpp,%.o,$(SRCS))
DEPS = $(patsubst %.cpp,%.d,$(SRCS))

DEPFLAGS = -MMD -MP -MF $*.d
CXXFLAGS += -O2 -fno-exceptions -fno-rtti -Wall -fno-strict-aliasing -I../include $(DEPFLAGS)

all: $(TARGET)

$(TARGET): $(OBJS)
	ar rc $@ $(OBJS)

-include $(DEPS)

.PHONY: clean
clean:
	rm -f $(TARGET) *.o *.d