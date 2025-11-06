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

/*
 * Lookup a host IP address and connect to it using service. Arguments match the
 * first two arguments to getaddrinfo(3).
 *
 * Returns a connected socket descriptor or -1 on error. Caller is responsible
 * for closing the returned socket.
 */
int
lookup_and_connect(const char* host, const char* service);

/* Sends data to the target socket, handling partial sends. */
int
safeSend(const int sockd, const char* buf, const ssize_t len);

/* Recieves data from the socket, handling partial recieves. */
int
safeRecv(const int sockd, char* buf, const ssize_t len);

/* Converts a 32-bit IP address (host byte order) to a string representation.
 * ipAddr in host byte order => converted with ntohl.
 * ipStr buffer must be MINIMUM => INET_ADDRSTRLEN bytes. 
 */
void
ipToString(uint32_t ipAddr, char* ipStr);

/* cConverts a 16-bit port number to a string representation.
 * portStr buffer must be at least 6 bytes (max "65535\0"). 
 */
void
portToString(uint16_t port, char* portStr);

#endif