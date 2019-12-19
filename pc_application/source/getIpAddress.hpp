#ifdef _WIN32
#include <Winsock2.h>
#else
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#endif

#include <cstdint>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

namespace GetIP {
// https://www.geeksforgeeks.org/c-program-display-hostname-ip-address/
// Driver code

bool getMyIP(char *ipAddress, int port) {
  // https://stackoverflow.com/a/122225
  // This is the hostname
  char szBuffer[1024];

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
  uint8_t b1 = (uint8_t)((struct in_addr *)(host->h_addr))->S_un.S_un_b.s_b1;
  uint8_t b2 = (uint8_t)((struct in_addr *)(host->h_addr))->S_un.S_un_b.s_b2;
  uint8_t b3 = (uint8_t)((struct in_addr *)(host->h_addr))->S_un.S_un_b.s_b3;
  uint8_t b4 = (uint8_t)((struct in_addr *)(host->h_addr))->S_un.S_un_b.s_b4;

#ifdef WIN32
  WSACleanup();
#endif

  // Copy into buffer (with port, for convinence)
  sprintf(ipAddress, "%d.%d.%d.%d:%d", b1, b2, b3, b4, port);

  return true;
}
} // namespace GetIP