#ifdef WIN32
#include <Winsock2.h>
#endif
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

namespace GetIP {
// https://www.geeksforgeeks.org/c-program-display-hostname-ip-address/
// Driver code

// Example: b1 == 192, b2 == 168, b3 == 0, b4 == 100
struct IPv4 {
  unsigned char b1, b2, b3, b4;
};

bool getMyIP(char *ipAddress) {
  // https://stackoverflow.com/a/122225
  GetIP::IPv4 myIP char szBuffer[1024];

#ifdef WIN32
  WSADATA wsaData;
  WORD wVersionRequested = MAKEWORD(2, 0);
  if (::WSAStartup(wVersionRequested, &wsaData) != 0)
    return false;
#endif

  if (gethostname(szBuffer, sizeof(szBuffer)) == SOCKET_ERROR) {
#ifdef WIN32
    WSACleanup();
#endif
    return false;
  }

  struct hostent *host = gethostbyname(szBuffer);
  if (host == NULL) {
#ifdef WIN32
    WSACleanup();
#endif
    return false;
  }

  // Obtain the computer's IP
  myIP.b1 = ((struct in_addr *)(host->h_addr))->S_un.S_un_b.s_b1;
  myIP.b2 = ((struct in_addr *)(host->h_addr))->S_un.S_un_b.s_b2;
  myIP.b3 = ((struct in_addr *)(host->h_addr))->S_un.S_un_b.s_b3;
  myIP.b4 = ((struct in_addr *)(host->h_addr))->S_un.S_un_b.s_b4;

#ifdef WIN32
  WSACleanup();
#endif

  // Copy into buffer
  sprintf(ipAddress, "%d:%d:%d:%d", myIP.b1, myIP.b2, myIP.b3, myIP.b4);
}
} // namespace GetIP