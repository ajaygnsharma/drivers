#include "cpp_c_ifc.h"

/* The goal is to show that C++ and C can inter-operate. If the signature of the
 * c++ file is compatible with C, then it will work. C does not understand
 * classes and so . operator will work with this. But otherwise simple names
 * will work.
 */
int main(int argc, char **argv) {
	_write(1);
	return 0;
}
