#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <netinet/in.h>
#include <sstream>
#include <string>
#include <thread>
#include <unistd.h>
#include <vector>

namespace fs = std::filesystem;

const int PORT = 80; // You can change to 80 with sudo
std::string www_dir = "./www";

void handle_client(int client_socket) {
  char buffer[4096] = {0};
  read(client_socket, buffer, 4096);

  std::istringstream request(buffer);
  std::string method, path, version;
  request >> method >> path >> version;

  std::cout << "Thread ID: " << std::this_thread::get_id() << ", Path: " << path
            << std::endl;

  // Default to index.html
  if (path == "/")
    path = "/index.html";

  // Sanitize path to prevent traversal
  fs::path requested_path = fs::weakly_canonical(fs::path(www_dir + path));
  fs::path base_path = fs::canonical(www_dir);

  if (requested_path.string().find(base_path.string()) != 0 ||
      !fs::exists(requested_path) || fs::is_directory(requested_path)) {
    std::string response =
        "HTTP/1.1 404 Not Found\r\n\r\n<h1>404 Not Found</h1>";
    send(client_socket, response.c_str(), response.size(), 0);
    close(client_socket);
    return;
  }

  std::ifstream file(requested_path);
  std::stringstream content;
  content << file.rdbuf();

  std::string response =
      "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n" + content.str();
  send(client_socket, response.c_str(), response.size(), 0);
  close(client_socket);
}

int main(int argc, char *argv[]) {
  if (argc > 1)
    www_dir = argv[1];

  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd == 0) {
    perror("socket failed");
    exit(EXIT_FAILURE);
  }

  int opt = 1;
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  sockaddr_in address;
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(PORT);

  if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
    perror("bind failed");
    exit(EXIT_FAILURE);
  }

  if (listen(server_fd, 10) < 0) {
    perror("listen");
    exit(EXIT_FAILURE);
  }

  std::cout << "Server listening on http://localhost:" << PORT << "/\n";

  while (true) {
    int addrlen = sizeof(address);
    int client_socket =
        accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
    if (client_socket < 0) {
      perror("accept");
      continue;
    }

    std::thread(handle_client, client_socket).detach(); // Handle in new thread
  }

  close(server_fd);
  return 0;
}
