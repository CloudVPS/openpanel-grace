all:
	cd src/libgrace && ./configure && make
	cd src/udbc && ./configure && make
	cd src/matrixssl && ./configure && make
	cd src/libgrace-ssl && ./configure && make

install: all
	cd src/libgrace && ./makeinstall
	cd src/udbc && ./makeinstall
	cd src/matrixssl && ./makeinstall
	cd src/libgrace-ssl && ./makeinstall

grace:
	cd src/libgrace && ./configure && make

docs:
	./mkdoc.sh

clean:
	cd src/libgrace && make clean
	cd src/udbc && make clean
	cd src/matrixssl && make clean
	cd src/libgrace-ssl && make clean
