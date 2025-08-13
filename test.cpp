#include "Atom.hpp"
#include <variant>


int main() {
	Atom c, d;
	Atom *a = new Atom;
    c = d;
    c = t{};
    
    d.marked = false;
    c.marked = true;
	*a=c;
	return (int)(c==d);
}
