PROJECT_ROOT = $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

all:	main

main: cpp_c_ifc.o main.o  
	g++ $(LDFLAGS) -o $@ $^

main.o:	main.c
	gcc -c $(CFLAGS) $(CPPFLAGS) -o $@ $<

cpp_c_ifc.o:	cpp_c_ifc.cpp
	g++ -c $(CXXFLAGS) $(CPPFLAGS) -o $@ $<
	
clean:
	rm -fr cpp_c_ifc main *.o
