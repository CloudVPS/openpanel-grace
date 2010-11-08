# This file is part of the Grace library (libgrace).
# The Grace library is free software: you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License as published by the
# Free Software Foundation, using version 3 of the License.
# You should have received a copy of the GNU Lesser General Public License 
# along with Grace library. If not, see <http://www.gnu.org/licenses/>.

all: mkconfigure

mkconfigure: mkconfigure.in
	./makemkconfigure
	chmod +x mkconfigure

clean:
	rm -f mkconfigure tests/tmp/*
	
install: all
	./makeinstall
