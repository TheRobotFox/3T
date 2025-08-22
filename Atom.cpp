#include "Atom.hpp"

namespace TTT {
	Header::Header(Header subtype, size_t length): data(ARRAY | markBit |
			   makeSize(sizeof(Array) + (length * subtype.getSize()))) {}

	
}
