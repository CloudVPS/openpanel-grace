all: mkconfigure

mkconfigure: mkconfigure.in
	./makemkconfigure
	chmod +x mkconfigure

clean:
	rm -f mkconfigure tests/tmp/*
	
install: all
	./makeinstall
