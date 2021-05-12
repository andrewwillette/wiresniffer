output: main.o
	g++ -g main.o -L/Users/aw052728/git/imtui/build/third-party -L/Users/aw052728/git/imtui/build/src -L/usr/local/Cellar/libtins/4.3/lib -limgui-for-imtui -limtui -limtui-ncurses -lncurses -ltins -o output 

main.o: main.cpp
	g++ -c main.cpp -I/Users/aw052728/git/imtui/include -I/Users/aw052728/git/imtui/include/imtui -I/usr/local/Cellar/libtins/4.3/include -o main.o -std=c++2a

clean:
	rm *.o output
