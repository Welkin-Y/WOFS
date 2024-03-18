all: wofs test

clean:
	rm -f wofs test

cflags:
	std=c++2a -Wall -Wextra -Wpedantic -Werror -O3

wofs: wofs.cpp log.c utils.cpp
	g++ -Wall $^ `pkg-config fuse --cflags --libs` -o $@

test: test.cpp log.c utils.cpp
	g++ -Wall $^ `pkg-config fuse --cflags --libs` -o $@

