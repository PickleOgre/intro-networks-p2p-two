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
using std::cout;
using std::endl;
using std::string;
using std::vector;

/*Peers use the JOIN action to notify the registry that they wish to participate
 * in the P2P network.  Peers should JOIN the P2P network by sending an
 * appropriate JOIN request to the registry before sending any other request.
 * Returns the socket descriptor, or -1 on error.*/
int
join(const char* host, const char* port, uint32_t peerID)
{
  int sockd;                          // Registry socket descriptor.
  uint32_t peerNetID = htonl(peerID); // Convert to network byte order.
  char req[5];                        // Buffer to hold message.

  // Construct the message.
  req[0] = 0x00;                                  // Action code: JOIN
  memcpy(req + 1, &peerNetID, sizeof(peerNetID)); // peerID is 4 bytes

  if ((sockd = lookup_and_connect(host, port)) < 0) {
    cerr << "Failed lookup and connect" << endl;
    return -1;
  }

  // Send message, and return success or failure.
  if (safeSend(sockd, req, 5) < 0)
    return -1;

  return sockd;
}

/* A peer uses the PUBLISH action to inform the registry about the files it has
 * available to share. Returns -1 on error or 0 otherwise.*/
int
publish(int sockd)
{
  DIR* dir;
  struct dirent* entry;
  vector<string> files;

  dir = opendir("SharedFiles");
  if (dir == NULL) {
    // no files, send empty publish.
    char req[5];
    req[0] = 0x01;
    uint32_t zero = 0;
    memcpy(req + 1, &zero, 4);

    if (safeSend(sockd, req, 5) < 0)
      return -1;
    return 0;
  }

  while ((entry = readdir(dir)) != NULL) {
    if (entry->d_type == DT_REG) {
      files.push_back(string(entry->d_name));
    }
  }
  closedir(dir);

  uint32_t count = files.size();
  size_t msgSize = 1 + 4;

  for (size_t i = 0; i < files.size(); i++) {
    msgSize += files[i].length() + 1;
  }

  char* req = new char[msgSize];
  size_t pos = 0;

  req[pos++] = 0x01;

  uint32_t countNet = htonl(count);
  memcpy(req + pos, &countNet, 4);
  pos += 4;

  for (size_t i = 0; i < files.size(); i++) {
    memcpy(req + pos, files[i].c_str(), files[i].length() + 1);
    pos += files[i].length() + 1;
  }

  int result = safeSend(sockd, req, msgSize);
  delete[] req;

  if (result < 0)
    return -1;

  return 0;
}

/*A peer uses the SEARCH action to locate another peer that contains a file of
 * interest.  Within the SEARCH request, the peer sends the file name the
 * registry should locate.  Search returns -1 on error and 0 otherwise.*/
int
search(int sockd, const string& filename)
{
  size_t msgSize = 1 + filename.length() + 1; // [Action][Filename][\0]
  char* req = new char[msgSize];              // Buffer to hold search request.

  // Construct the search request message.
  req[0] = 0x02; // Action code: SEARCH
  memcpy(req + 1, filename.c_str(), msgSize - 1);

  if (safeSend(sockd, req, msgSize) < 0) {
    delete[] req;
    return -1;
  }
  delete[] req;

  // Get response from registry
  char response[10];
  if (safeRecv(sockd, response, 10) < 10) {
    cerr << "Error receiving search response" << endl;
    return -1;
  }

  // Parse response
  uint32_t peerID, ipAddr;
  uint16_t port;
  memcpy(&peerID, response, 4);
  memcpy(&ipAddr, response + 4, 4);
  memcpy(&port, response + 8, 2);
  // Ensure host byte order
  peerID = ntohl(peerID);
  ipAddr = ntohl(ipAddr);
  port = ntohs(port);

  // If all fields == 0, file is not indexed by registry
  if (peerID == 0 && ipAddr == 0 && port == 0) {
    cout << "File not indexed by registry" << endl;
    return 0;
  }

  // convert for print
  struct in_addr addr;
  addr.s_addr = htonl(ipAddr);
  char ipStr[INET_ADDRSTRLEN];
  inet_ntop(AF_INET, &addr, ipStr, INET_ADDRSTRLEN);

  // Output
  cout << "File found at" << endl;
  cout << "Peer " << peerID << endl;
  cout << ipStr << ":" << port << endl;

  return 0;
}

#endif