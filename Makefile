homeFolder = /Users/andrewwillette

output: main.o
	g++ -g main.o -L$(homeFolder)/git/imtui/build/third-party -L$(homeFolder)/git/imtui/build/src -L/usr/local/Cellar/libtins/4.3/lib -limgui-for-imtui -limtui -limtui-ncurses -lncurses -ltins -o output 

main.o: main.cpp
	g++ -c main.cpp -I$(homeFolder)/git/imtui/include -I$(homeFolder)/git/imtui/include/imtui -I/usr/local/Cellar/libtins/4.3/include -o main.o -std=c++2a

clean:
	rm *.o output
