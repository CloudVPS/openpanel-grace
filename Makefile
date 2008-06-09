all:
	cd src/libgrace && make
	cd src/libgrace-pcre && make
	cd src/libdbfile && make
	cd src/matrixssl && make
	cd src/libgrace-ssl && make
	cd src/libquerido && make

install: all
	cd src/libgrace && ./makeinstall
	cd src/matrixssl && ./makeinstall
	cd src/libgrace-ssl && ./makeinstall
	cd src/libdbfile && ./makeinstall
	cd src/libquerido && ./makeinstall
	cd src/libgrace-pcre && ./makeinstall

clean:
	cd src/libgrace && make clean
	cd src/libgrace-pcre && make clean
	cd src/libdbfile && make clean
	cd src/libgrace-ssl && make clean
	cd src/libquerido && make clean

grace:
	cd src/libgrace && ./configure && make

docs:
	./mkdoc.sh
