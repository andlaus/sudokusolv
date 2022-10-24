PROJECT_DIR=.

SOURCES=$(shell echo *.cc)
OBJECTS=$(subst .cc,.o,$(SOURCES))
HEADERS=$(shell echo *.hh)
TARGET=sudokusolv

CXX=g++
CXXFLAGS=-std=c++17 -Wall -O3 -march=native
#CXXFLAGS=-std=c++14 -Wall -g -Og
#CXXFLAGS=-std=c++14 -Wall -g

LD=g++
LDFLAGS=-lm

all: $(TARGET);

.o: .cc
	$(CXX) $(CXXFLAGS) -o $@ $?

$(TARGET): $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $?

clean:
	rm -rf $(OBJECTS)
	rm -rf $(TARGET)
