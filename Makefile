output: main.o
	g++ -g main.o -L/Users/aw052728/git/imtui/build/third-party -limgui-for-imtui -L/Users/aw052728/git/imtui/build/src -limtui -limtui-ncurses -lncurses -o output 

main.o: main.cpp
	g++ -c main.cpp -I/Users/aw052728/git/imtui/include -I/Users/aw052728/git/imtui/include/imtui -o main.o -std=c++2a

clean:
	rm *.o output