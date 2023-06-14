run: build
	./server 8000

build:
	gcc server.c -o server

clean:
	rm server
