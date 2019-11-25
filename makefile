all: bin/sisctrl

CXXFLAGS=-g
LDFLAGS= -std=c++11 -pthread -L /usr/local/lib -lyaml-cpp 

# INVOICE_OBJECTS=src/sisctrl.o src/automata.o
INVOICE_OBJECTS=src/sisctrl.o


bin/sisctrl: $(INVOICE_OBJECTS)
	$(CXX) -o $@ $^  $(LDFLAGS)

src/sisctrl.o: src/sisctrl.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# src/automata.o: src/automata.cpp src/automata.h
# 	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f src/*.o src/*.cpp~ Makefile~