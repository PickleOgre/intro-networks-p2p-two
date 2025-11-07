# This makefile is for testing and submission, as the script does not allow multiple directories.
peer: main.cpp net_tools.h net_tools.cpp p2p_tools.h p2p_tools.cpp
	g++ -std=c++11 -Wall main.cpp net_tools.cpp p2p_tools.cpp -o peer

clean:
	rm -f peer
