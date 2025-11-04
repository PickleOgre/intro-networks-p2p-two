peer: main.cpp net_tools.h p2p_tools.h
	g++ -std=c++11 -Wall main.cpp -o peer

clean:
	rm -f peer
