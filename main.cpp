#include <iostream>
#include <sstream>
#include <string>

#include "socklib.h"

void do_client();
void do_server();

int main(int argc, char *argv[]) {
  if (argc == 1) do_client();
  else do_server();

  return 0;
}

void do_client() {
  Socket connection(Socket::Family::INET, Socket::Type::STREAM);
  
  Address addr("127.0.0.1", 7778);

  std::cout << "[Client] connecting...\n";
  connection.Connect(addr);

  std::cout << "[Client] connected.\n";
  std::string to_send("Hi server!");
  std::cout << "[Client] sending...\n";
  size_t nbytes_sent = connection.Send(to_send.data(), to_send.size());

  std::cout << "[Client] sent " << nbytes_sent << " bytes.\n";
}

void do_server() {
  std::cout << "[Server] running...\n";
  Socket listen_sock(Socket::Family::INET, Socket::Type::STREAM);
  Address addr("127.0.0.1", 7778);

  listen_sock.Bind(addr);

  listen_sock.Listen();

  std::cout << "[Server] accepting...\n";
  Socket conn_sock = listen_sock.Accept();

  std::cout << "[Server] conn received...\n";

  char buffer[4096];

  std::cout << "[Server] receiving...\n";
  size_t nbytes_received = conn_sock.Recv(buffer, sizeof(buffer));
  
  std::cout << "Received message '" << std::string(buffer, nbytes_received) << "'\n";
}

std::string build_string(std::istream &in_stream) {
  /* TODO (optional): implement this function */
  return std::string();
}

