#include "Atom.hpp"
#include <variant>


struct structt {
	int a;
	
};
int main() {
	Atom c, d;
	Atom *a = new Atom;
	c = d;
	*a=c;
	return (int)(c==d);
}
