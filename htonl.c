#include <arpa/inet.h>
#include <stdio.h>

int main() {
  // Define a 32-bit integer in host byte order (assuming your system is little-endian)
  uint32_t host_long = 0x12345678;  // Represents 305419896 in decimal

  // Convert the integer to network byte order using htonl
  uint32_t network_byte_order = htonl(host_long);

  // Print the host byte order and network byte order representations
  printf("Host Byte Order (little-endian): 0x%x\n", host_long);
  printf("Network Byte Order:              0x%x\n", network_byte_order);

  return 0;
}
