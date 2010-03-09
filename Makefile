all:
	cd src/grace-configure && make
	cd src/libgrace && make
	cd src/libgrace-pcre && make
	cd src/libdbfile && make
	cd src/matrixssl && make
	cd src/libgrace-ssl && make
	cd src/libquerido && make

install: all
	cd src/grace-configure && make install
	cd src/libgrace && ./makeinstall
	cd src/matrixssl && ./makeinstall
	cd src/libgrace-ssl && ./makeinstall
	cd src/libdbfile && ./makeinstall
	cd src/libquerido && ./makeinstall
	cd src/libgrace-pcre && ./makeinstall

clean:
	cd src/libgrace && make clean || true
	cd src/libgrace-pcre && make clean || true
	cd src/libdbfile && make clean || true
	cd src/libgrace-ssl && make clean || true
	cd src/libquerido && make clean || true

grace:
	cd src/libgrace && ./configure && make

docs:
	./mkdoc.sh
