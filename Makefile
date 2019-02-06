CXXFLAGS = -I. -Wall -Wextra -Wpedantic -std=c++11

SPEC_SOURCES = $(wildcard spec/*.cpp)
SPEC_OBJECTS = $(addprefix build/, $(SPEC_SOURCES:.cpp=.o))
SPEC_BINARY = bin/spec

.PHONY: test clean

all: test

test: $(SPEC_BINARY)
	$(SPEC_BINARY)

$(SPEC_BINARY): $(SPEC_OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

build/%.o: %.cpp staticset.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(SPEC_OBJECTS) $(SPEC_BINARY)
