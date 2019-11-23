all: sisctrl

sisctrl: src/sisctrl.o 
	$(CXX) -o $@ $^



clean:
	rm src/*.o