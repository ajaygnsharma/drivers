CXX=g++
LD=g++
CXXFLAGS=-g -O2
LDFLAGS=
LIBS=-lz 

program: program.o
	$(LD) -o $@ $^ $(LDFLAGS) $(LIBS)
	
.PHONY: clean

clean:
	rm -f program *.o
	
