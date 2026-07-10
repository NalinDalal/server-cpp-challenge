// Include the client handler implementation
#include "client.cpp"
// Standard C/C++ headers
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <string>
#include <thread>
#include <unistd.h>

// Server configuration constants
const int PORT = 80;           // Port to listen on (use sudo for port 80)
std::string www_dir = "./www"; // Root directory for serving files

int main(int argc, char *argv[]) {
  // Allow overriding the www directory via command-line argument
  if (argc > 1)
    www_dir = argv[1];

  // Create a TCP socket (IPv4, stream-oriented)
  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd == 0) { // handle error if no socket is created
    perror("socket failed");
    exit(EXIT_FAILURE);
  }

  // Allow reuse of the address to avoid "address already in use" errors
  int opt = 1;
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  // Bind the socket to all network interfaces on the specified port
  sockaddr_in address;
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(PORT);

  // if no binding happens b/w the given server address and the port
  if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
    perror("bind failed");
    exit(EXIT_FAILURE);
  }

  // Start listening for incoming connections with a backlog of 10
  if (listen(server_fd, 10) < 0) {
    perror("listen");
    exit(EXIT_FAILURE);
  }

  std::cout << "Server listening on http://localhost:" << PORT << "/\n";

  // Main accept loop: handle each client in a new detached thread
  while (true) {
    int addrlen = sizeof(address); // length of address structure
    int client_socket =
        accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
    if (client_socket < 0) {
      perror("accept");
      continue;
    }

    std::thread(handle_client, client_socket).detach();
    // create a new thread for each client
  }

  close(server_fd);
  return 0;
}
