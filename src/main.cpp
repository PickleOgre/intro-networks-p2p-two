/* Group Members: Joe Lawrence, Nate Smith
 * Class: EECE 446, Introduction to Computer Networking
 * Semester: Fall 2025 */

#include "net_tools.h"
#include "p2p_tools.h"
#include <iostream>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <string>
using std::stoul;
using std::cout;
using std::cin;
using std::string;
using std::cerr;
using std::endl;

int
main(int argc, char* argv[])
{
  // Check for errors in command line args.
  if (argc < 4) {
    cerr << "Usage: " << argv[0]
         << " <registry location> <registry port#> <peer id>" << endl;
    exit(1);
  }

  // Declare network info variables
  const char *host = argv[1];
  const char *port = argv[2];
  uint32_t peerID = stoul(argv[3]);
  int sockd = 0;

  // Declare program control variables.
  bool running = true;
  string cmd;

  // Until the user exits the program, loop the command UI.
  while (running)
  {
    // Read in command from user
    cout << "Enter a command: ";
    cin >> cmd;

    // Execute appropriate code for command entered.
    if (cmd == "JOIN"){
      sockd = join(host, port, peerID); 
      if (sockd < 0) {
        cerr << "JOIN failed" << endl;
      }
    } else if (cmd == "PUBLISH") {
      if (publish(sockd) < 0) {
        cerr << "PUBLISH failed" << endl;
      }
    } else if (cmd == "SEARCH") {
      cout << "Enter a file name: ";
      string filename;
      cin >> filename;
      if (search(sockd, filename) < 0) {
        cerr << "SEARCH failed" << endl;
      }
    } else if (cmd == "EXIT") {
      close(sockd);
      running = false;
    } else {
      cout << "Commands: JOIN, PUBLISH, SEARCH, EXIT" << endl;
      continue;
    }
  }
}
