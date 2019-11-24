all: src/sisctrl

CXXFLAGS=-g
LDFLAGS=-L /usr/local/lib -lyaml-cpp

# INVOICE_OBJECTS=src/sisctrl.o src/automata.o
INVOICE_OBJECTS=src/sisctrl.o


src/sisctrl: $(INVOICE_OBJECTS)
	$(CXX) -o $@ $^  $(LDFLAGS)

src/sisctrl.o: src/sisctrl.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# src/automata.o: src/automata.cpp src/automata.h
# 	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f src/*.o src/*.cpp~ Makefile~