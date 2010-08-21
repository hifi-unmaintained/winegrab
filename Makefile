# release version
REL=0.3

all:
	gcc -Wall -g -O2 -o winegrab main.c -lX11

clean:
	rm -f winegrab

realclean: clean
