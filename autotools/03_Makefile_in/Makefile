CXX=g++
LD=g++
CXXFLAGS=-g -O2

hello: hello.o
	$(LD) -o $@ $^

.PHONY: clean
clean:
	rm -f hello *.o
