SHELL	     = /bin/sh
CC	     = g++
FLAGS	     = -std=c++11 
CFLAGS	     = -Wall 
DEBUGFLAGS   = -O0 -g
RELEASEFLAGS = -O2
TARGET	     = shmup
SOURCES	     = $(shell echo src/*.cpp)
HEADERS	     = $(shell echo include/*.h)
OBJECTS	     = $(SOURCES:.cpp=.o)
LIBCFLAGS    = $(shell pkg-config sfml-all  --cflags) 
LIBS	     = $(shell pkg-config sfml-all  --libs) 

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(FLAGS) $(CFLAGS) $(DEBUGFLAGS) -o $(TARGET) $(OBJECTS) $(LIBS)

release: $(SOURCES) $(HEADERS)
	$(CC) $(FLAGS) $(CFLAGS) $(RELEASEFLAGS) $(LIBCFLAGS) $(LIBS) -o $(TARGET) $(SOURCES)

clean:
	-rm -f $(OBJECTS)

distclean: clean
	-rm -f $(TARGET)

.SECONDEXPANSION:

$(foreach OBJ,$(OBJECTS),$(eval $(OBJ)_DEPS = $(shell $(CC) -MM $(OBJ:.o=.cpp) | sed s/.*://)))
%.o: %.cpp $$($$@_DEPS)
	$(CC) $(FLAGS) $(CFLAGS) $(DEBUGFLAGS) $(LIBCFLAGS) -c -o $@ $<

.PHONY : all release \
  clean distclean
