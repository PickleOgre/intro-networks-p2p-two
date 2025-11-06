#ifndef P2P_TOOLS_H
#define P2P_TOOLS_H

/* Group Members: Joe Lawrence, Nate Smith
 * Class: EECE 446, Introduction to Computer Networking
 * Semester: Fall 2025 */

/* This library contains the code for our P2P functions, including JOIN,
 * PUBLISH, SEARCH, and EXIT.*/

#include "net_tools.h"
#include <arpa/inet.h>
#include <dirent.h>
#include <iostream>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>
#include <fstream>

/*Peers use the JOIN action to notify the registry that they wish to participate
 * in the P2P network.  Peers should JOIN the P2P network by sending an
 * appropriate JOIN request to the registry before sending any other request.
 * Returns the socket descriptor, or -1 on error.*/
int
join(const char* host, const char* port, uint32_t peerID);

/* A peer uses the PUBLISH action to inform the registry about the files it has
 * available to share. Returns -1 on error or 0 otherwise.*/
int
publish(int sockd);

/*A peer uses the SEARCH action to locate another peer that contains a file of
 * interest.  Within the SEARCH request, the peer sends the file name the
 * registry should locate.  Search returns -1 on error and 0 otherwise.*/
int
search(int sockd, const std::string& filename);

/* Peers use the FETCH action to request a file from another peer in the P2P
 * network.  Peers must JOIN (or REGISTER to) the P2P network before sending a
 * FETCH request. */
int
fetch(std::string filename);

#endif