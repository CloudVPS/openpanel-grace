all: mkconfigure

mkconfigure: mkconfigure.in
	./makemkconfigure
	chmod +x mkconfigure

clean:
	rm -f mkconfig tests/tmp/*
	
install: all
	./makeinstall

test: mkconfigure
	mkdir -p tests/tmp
	cd tests/tmp && rm -f * && echo fake1 > configure.in && ../../mkconfigure ; ./configure > stdout 2> stderr ; ../compare ../fake1
	cd tests/tmp && rm -f * && echo fakefail > configure.in && ../../mkconfigure ; ./configure > stdout 2> stderr ; ../compare ../fakefail
	cd tests/tmp && rm -f * && echo fake1 > configure.in && ../../mkconfigure ; ./configure -q > stdout 2> stderr ; ../compare ../fake1q
	cd tests/tmp && rm -f * && echo fakefail > configure.in && ../../mkconfigure ; ./configure -q > stdout 2> stderr ; ../compare ../fakefailq
	@echo
	@echo Output verified and good!
