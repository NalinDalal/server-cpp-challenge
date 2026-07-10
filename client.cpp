// Filesystem and I/O headers for file serving
#include <filesystem>
#include <fstream>
#include <sstream>
#include <vector>

namespace fs = std::filesystem;

// Handle an individual HTTP client request in its own thread
void handle_client(int client_socket) {
  // Read the raw HTTP request from the client socket
  char buffer[4096] = {0};
  read(client_socket, buffer, 4096);

  // Parse the HTTP request line (e.g., "GET /index.html HTTP/1.1")
  std::istringstream request(buffer);
  std::string method, path, version;
  request >> method >> path >> version;

  std::cout << "Thread ID: " << std::this_thread::get_id() << ", Path: " << path
            << std::endl;

  // Default to index.html when the root path is requested
  if (path == "/")
    path = "/index.html";

  // Resolve the requested path and ensure it stays within the www root
  // fs::weakly_canonical resolves ".." and symlinks to prevent directory traversal
  fs::path requested_path = fs::weakly_canonical(fs::path(www_dir + path));
  fs::path base_path = fs::canonical(www_dir);

  // Return 404 if the path is outside the base directory, doesn't exist, or is a directory
  if (requested_path.string().find(base_path.string()) != 0 ||
      !fs::exists(requested_path) || fs::is_directory(requested_path)) {
    std::string response =
        "HTTP/1.1 404 Not Found\r\n\r\n<h1>404 Not Found</h1>";
    send(client_socket, response.c_str(), response.size(), 0);
    close(client_socket);
    return;
  }

  // Read the requested file and send it as the HTTP response body
  std::ifstream file(requested_path);
  std::stringstream content;
  content << file.rdbuf();

  std::string response =
      "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n" + content.str();
  send(client_socket, response.c_str(), response.size(), 0);
  close(client_socket);
}
