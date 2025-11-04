#ifndef NET_TOOLS_H
#define NET_TOOLS_H

/* Group Members: Joe Lawrence, Nate Smith
 * Class: EECE 446, Introduction to Computer Networking
 * Semester: Fall 2025 */

/* These functions are useful for general network programming. Prioritize using
 * safeSend() and SafeRecv() over native functions when possible*/

#include <errno.h>
#include <iostream>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
using std::cerr;
using std::endl;

/*
 * Lookup a host IP address and connect to it using service. Arguments match the
 * first two arguments to getaddrinfo(3).
 *
 * Returns a connected socket descriptor or -1 on error. Caller is responsible
 * for closing the returned socket.
 */
int
lookup_and_connect(const char* host, const char* service)
{
  struct addrinfo hints;
  struct addrinfo *rp, *result;
  int s;

  /* Translate host name into peer's IP address */
  // avoid using memset per assignment requirements
  hints = { 0 };
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = 0;
  hints.ai_protocol = 0;

  if ((s = getaddrinfo(host, service, &hints, &result)) != 0) {
    fprintf(stderr, "stream-talk-client: getaddrinfo: %s\n", gai_strerror(s));
    return -1;
  }

  /* Iterate through the address list and try to connect */
  for (rp = result; rp != NULL; rp = rp->ai_next) {
    if ((s = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) == -1) {
      continue;
    }

    if (connect(s, rp->ai_addr, rp->ai_addrlen) != -1) {
      break;
    }

    close(s);
  }

  if (rp == NULL) {
    perror("stream-talk-client: connect");
    return -1;
  }
  freeaddrinfo(result);

  return s;
}

/* Sends data to the target socket, handling partial sends. */
int
safeSend(const int sockd, const char* buf, const ssize_t len)
{
  if (len <= 0) { // Sanity check
    cerr << "len cannot be negative" << endl;
    return -1;
  }
  // Send the request to the server. Keep sending until the full message has
  // been transmitted.
  ssize_t total_bytes_sent = 0; // Keep track of how much data we have sent in
                                // total.

  while (total_bytes_sent < len) // Loop until we have sent the full string
  {
    ssize_t bytes_sent = 0; // The amount of bytes sent in each loop
    bytes_sent = send(sockd,
                      buf + total_bytes_sent,
                      len - total_bytes_sent,
                      0); // fixed pointer bug
    if (bytes_sent < 0 &&
        (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)) {
      // Interrupted by signal or socket temporarily unavailable -> safe to
      // retry immediately
      continue;
    } else if (bytes_sent == 0) {
      // Connection closed during send -> break
      cerr << "Connection closed" << endl;
      break;
    } else if (bytes_sent < 0) {
      // Some real error occurred -> return error
      cerr << "Error during send" << endl;
      return -1;
    } else {
      total_bytes_sent += bytes_sent;
    }
  }
  return total_bytes_sent;
}

/* Recieves data from the socket, handling partial recieves. */
int
safeRecv(const int sockd, char* buf, const ssize_t len)
{
  if (len <= 0) {
    cerr << "safeRecv: invalid length" << endl;
    return -1;
  }

  ssize_t total_bytes_received = 0; // number of bytes recieved since start
  while (total_bytes_received < len) {
    ssize_t bytes_received = 0; // number of bytes recieved in one recv()
    // Recieve up to len bytes of data from server
    // Ask for more if client doesn't get len bytes.
    // Stop early if the server terminates early.
    bytes_received =
      recv(sockd, buf + total_bytes_received, len - total_bytes_received, 0);
    if (bytes_received > 0) {
      // Keep track of building the message.
      total_bytes_received += bytes_received;
    } else if (bytes_received == 0) {
      // Orderly shutdown - server has closed the socket.
      // Exit inner loop, and let socket_open = false so the outer loop
      // completes and exits.
      break;
    } else if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK) {
      continue; // Safe to retry
    } else {
      // Error occurred
      cerr << "Error during recv" << endl;
      return bytes_received;
    }
  }
  return total_bytes_received; // Success
}

#endif