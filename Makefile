all: test_prg_client test_prg client server

server:server_daemons.cpp
	g++ --std=c++11 -g server_daemons.cpp -o server -lrt

client:client_daemon.cpp
	g++ --std=c++11 -g client_daemon.cpp -o client -lrt

test_prg_client: test_prg_client.cpp libmap.a goldchase.h
	g++ --std=c++11 -g test_prg_client.cpp -o test_prg_client -L. -lmap -lpanel -lncurses -lrt -lpthread

test_prg: test_prg.cpp libmap.a goldchase.h
	g++ --std=c++11 -g test_prg.cpp -o test_prg -L. -lmap -lpanel -lncurses -lrt -lpthread

libmap.a: Screen.o Map.o
	ar -r libmap.a Screen.o Map.o

Screen.o: Screen.cpp
	g++ --std=c++11 -c Screen.cpp

Map.o: Map.cpp
	g++ --std=c++11 -c Map.cpp

clean:
	rm -f Screen.o Map.o libmap.a test_prg test_prg_client client server goldchase
