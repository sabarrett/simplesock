#include <iostream>
#include <sstream>
#include <string>
#include <time.h>

#include "socklib.h"

void do_client();
int do_server();

int main(int argc, char *argv[]) {
  SockLibInit();
  int result = do_server();
  SockLibShutdown();
  return result;
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

int do_server() {
  std::cout << "[Server] running...\n";
  Socket conn_sock(Socket::Family::INET, Socket::Type::DGRAM);

  char buffer[4096];

  std::cout << "[Server] receiving...\n";
  Address addr("127.0.0.1", 7778);
  conn_sock.Bind(addr);

  Address from_addr;

  float max_timeout = 5;
  float timeout = 1.2;
  int nbytes_received = 0;
  bool recv_failed = false;
  conn_sock.SetTimeout(timeout);

  time_t before_retry = time(NULL);

  while (timeout < max_timeout) {
    nbytes_received = conn_sock.RecvFrom(buffer, sizeof(buffer), from_addr);
    if (nbytes_received == -1 && conn_sock.GetLastError() == Socket::Error::SOCKLIB_EWOULDBLOCK) {
      std::cout << "Timeout occurred.\n";
      timeout *= 2;
      if (timeout >= max_timeout) {
	std::cout << "Recv() failed.\n";
	recv_failed = true;
      } else {
	std::cout << "Retrying with timeout=" << timeout << "...\n";
	conn_sock.SetTimeout(timeout);
      }
    }
  }

  time_t after_retry = time(NULL);
  time_t seconds_passed = after_retry - before_retry;
  if (!recv_failed) {
    std::cout << "Recv succeeded for some reason?\n";
    return 1;
  }
  if (seconds_passed < 7) {
    std::cout << seconds_passed << " seconds spent retrying,"
	      << "but should have been at least 7\n";
    return 1;
  }
  std::cout << seconds_passed << " seconds spent retrying. Success.\n";
  return 0;
  
  std::cout << "[Server] received message '" << std::string(buffer, nbytes_received)
	    << "' from host '" << from_addr << "'\n";
  return 1;
}

std::string build_string(std::istream &in_stream) {
  /* TODO (optional): implement this function */
  return std::string();
}

