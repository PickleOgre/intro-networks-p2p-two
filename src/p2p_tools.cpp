/* Group Members: Joe Lawrence, Nate Smith
 * Class: EECE 446, Introduction to Computer Networking
 * Semester: Fall 2025 */

#include "p2p_tools.h"
using std::cerr;
using std::cout;
using std::endl;
using std::string;
using std::vector;

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
    std::cerr << "Failed lookup and connect" << endl;
    return -1;
  }

  // Send message, and return success or failure.
  if (safeSend(sockd, req, 5) < 0)
    return -1;

  return sockd;
}

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
    std::cerr << "Error receiving search response" << endl;
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
  char ipStr[INET_ADDRSTRLEN];
  ipToString(ipAddr, ipStr);

  // Output
  cout << "File found at" << endl;
  cout << "Peer " << peerID << endl;
  cout << ipStr << ":" << port << endl;

  return 0;
}

int
fetch(int sockd, const string& filename)
{
  //1 (action code) + (file) + 1 (endl)
  size_t msgSize = 1 + filename.length() + 1;
  char* req = new char[msgSize];
  req[0] = 0x02;
  memcpy(req + 1, filename.c_str(), msgSize - 1);
  if ( safeSend(sockd, req, msgSize) < 0 ) 
  {
    delete[] req;
    return -1;
  }
  delete[] req;
  char response[10]; //4bytes peer + 4bytes ip + 2bytes port
  if ( safeRecv(sockd, response, 10) < 10 ) //essentially a hard coded search (should make into a more robust func for p4 but will take some refactoring)
  {
    cerr << "Err receiving search response" << endl;
    return -1;
  }

  uint32_t peerID, ipAddr; //vars for response feild (can split this section & onwards into helper func latr)
  uint16_t port;
  memcpy(&peerID, response, 4); // peer id 0-3
  memcpy(&ipAddr, response + 4, 4); // ip 4-7
  memcpy(&port, response + 8, 2); // 8-9 port
  peerID = ntohl(peerID);
  ipAddr = ntohl(ipAddr);
  port   = ntohs(port);

  if (peerID == 0 && ipAddr == 0 && port == 0) { //handle file not found technically this is a succes
    cout << "File not indexed by registry" << endl;
    return 0;
  }

  char ipStr[INET_ADDRSTRLEN];
  char portStr[6];
  ipToString(ipAddr, ipStr);
  portToString(port, portStr);

  int peerSockd = lookup_and_connect(ipStr, portStr);
  if (peerSockd < 0) {
    cerr << "Failed to connect to peer" << endl;
    return -1;
  }

  size_t fetchMsgSize = 1 + filename.length() + 1;
  char* fetchReq = new char[fetchMsgSize];
  fetchReq[0] = 0x03;
  memcpy(fetchReq + 1, filename.c_str(), fetchMsgSize - 1);

  if (safeSend(peerSockd, fetchReq, fetchMsgSize) < 0) 
  {
    delete[] fetchReq;
    close(peerSockd);
    return -1;
  }
  //cleanup
  delete[] fetchReq;

  /*rest of fetch -> TODO
  
  
  */
  close(peerSockd);
  return 0;
  
fetch(string filename)
{

  // Test Vars
  const char* file_name = "some_file.txt";

  // Recieve and save
  int sockd = 0;
  const size_t len = 1024;
  char buf[len];
  std::ofstream output_file(file_name, std::ios::binary);

  bool socket_open = true;
  while (socket_open == true) {
    
    ssize_t bytes_received = 0; // The amount of bytes that have been recieved
                                // since the start of the buffer.
    bytes_received = safeRecv(sockd, buf, len);
    if (bytes_received > 0) {
      output_file.write(buf, bytes_received);
      output_file.flush();
    } else if (bytes_received == 0) {
      socket_open = false;
    }
  }

  output_file.close();
}