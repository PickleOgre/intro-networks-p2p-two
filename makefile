peer: src/main.cpp src/net_tools.h src/net_tools.cpp src/p2p_tools.h src/p2p_tools.cpp
	g++ -std=c++11 -Wall src/main.cpp src/net_tools.cpp src/p2p_tools.cpp -o peer

clean:
	rm -f peer