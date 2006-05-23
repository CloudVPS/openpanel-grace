all:
	./configure
	./build
	cp *.h src/*.h ../../include/matrixssl/
	cat matrixCommon.h | sed -e "s@\"src/matrixConfig.h\"@<matrixssl/matrixConfig.h>@" > ../../include/matrixssl/matrixCommon.h
	cp src/libmatrixsslstatic.a ../../lib/
install:
	./makeinstall
