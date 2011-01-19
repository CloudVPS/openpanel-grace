#include <grace/application.h>
#include <grace/xmlschema.h>

int main (const char**argv,int argc)
{
	value data;
	
	data.loadxml ( argc>1 ? argv[1] : "in.xml");
	return 0;
}

