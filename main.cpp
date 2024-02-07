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
  Socket send_sock(Socket::Family::INET, Socket::Type::DGRAM);
  
  Address addr("127.0.0.1", 7778);
  Address my_addr("127.0.0.1", 52000);
  send_sock.Bind(my_addr);

  std::string to_send("Hi server!");
  std::cout << "[Client] sending to '" << addr << "'...\n";
  size_t nbytes_sent = send_sock.SendTo(to_send.data(), to_send.size(), addr);

  std::cout << "[Client] sent " << nbytes_sent << " bytes.\n";
}

void do_server() {
  std::cout << "[Server] running...\n";
  Socket conn_sock(Socket::Family::INET, Socket::Type::DGRAM);

  char buffer[4096];

  std::cout << "[Server] receiving...\n";
  Address addr("127.0.0.1", 7778);
  conn_sock.Bind(addr);

  Address from_addr;

  size_t nbytes_received = conn_sock.RecvFrom(buffer, sizeof(buffer), from_addr);
  
  std::cout << "[Server] received message '" << std::string(buffer, nbytes_received)
	    << "' from host '" << from_addr << "'\n";
}

std::string build_string(std::istream &in_stream) {
  /* TODO (optional): implement this function */
  return std::string();
}

