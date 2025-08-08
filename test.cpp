#include "Atom.hpp"
#include <variant>


struct structt {
	int a;
	
};
int main() {
	Atom c,d;
	c=d;
	return (int)(c==d);
}
