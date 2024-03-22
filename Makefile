all: wofs test

clean:
	rm -f wofs test

wofs: wofs.cpp log.c utils.cpp
	g++ -Wall $^ `pkg-config fuse --cflags --libs` -o $@

test: test.cpp log.c utils.cpp
	g++ -Wall $^ `pkg-config fuse --cflags --libs` -o $@

