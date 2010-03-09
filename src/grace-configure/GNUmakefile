all: mkconfigure

mkconfigure: mkconfigure.in
	./makemkconfigure
	chmod +x mkconfigure

clean:
	rm -f mkconfig tests/tmp/*
	
install: all
	./makeinstall
