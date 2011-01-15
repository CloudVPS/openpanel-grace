#!/usr/bin/env python
from __future__ import with_statement

import msgpack


testdata = [
	"short string",
	"string of more than 31 bytes in length, which should triggering the 0xda opcode.",
	None,
	True,
	False,
	0,
	1,-1,
	128,-128,
	65000,-65000,
	3.1415926,
	{"a":1,"b":2}, 
	{1:"a",2:"b"}, 
	[3,0,1,3],
	range(0,40),
	{}, 
	[],
]

with open("bigtest.msgpack", "wb") as f:
	f.write( msgpack.packb(testdata) )


#with open("int.msgpack", "wb") as f:
#	f.write( msgpack.packb(42) )

#with open("string.msgpack", "wb") as f:
#	f.write( msgpack.packb("Hello World!") )

#with open("array.msgpack", "wb") as f:
#	f.write( msgpack.packb([-1,-40,240,67000,-67000]) )

#with open("map.msgpack", "wb") as f:
#	f.write( msgpack.packb( { "November": 11, "February":2 }) )

