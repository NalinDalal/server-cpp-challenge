#ifndef CLIENT_H
#define CLIENT_H

#include <string>

extern std::string www_dir;

void handle_client(int client_socket);

#endif
