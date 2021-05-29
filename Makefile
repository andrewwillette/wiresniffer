output: main.o
	g++ -g main.o -L${HOME}/git/imtui/build/third-party -L${HOME}/git/imtui/build/src -L/usr/local/Cellar/libtins/4.3/lib -limgui-for-imtui -limtui -limtui-ncurses -lncurses -ltins -lfmt -std=c++2a -stdlib=libc++ -o output 

main.o: main.cpp
	g++ -c main.cpp -I${HOME}/git/imtui/include -I${HOME}/git/imtui/include/imtui -I/usr/local/Cellar/libtins/4.3/include -o main.o -stdlib=libc++ -std=c++2a

clean:
	rm *.o output
