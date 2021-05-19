#include "ident.h"
#include <cassert> 

int main()
{
	{
		Ident ident;
		//Ident ident2;	// bad idea, mutex is busy

		ident.set("C2-A2-B7");
		assert(ident.inc() == "C2-A2-B8");

		ident.set("B5-Z9");
		assert(ident.inc() == "B6-A1");

		ident.set("K2-I9-Z9");
		assert(ident.inc() == "K2-K1-A1");
	}
	// leave instance scope and free mutex
	Ident ident2;	// Ok
}
