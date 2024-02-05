#include <cstdio>
#include <iostream>
#include <ostream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>

#include <string.h>

#include "allocators.h"
#include "defer.h"
#include "socklib.h"

std::string do_client(std::istream &in_stream);
int run_test();

int main(int argc, char *argv[]) {
  /* TODO: ADD SETUP CODE HERE */
  SockLibInit();
  defer _shutdown_socklib([]() { SockLibShutdown(); });

  return run_test();
}

int run_test() {
  std::stringstream input("30\n500\n-12\n3.6\n"
			  "not a number\n200also not a number\n"
			  "88.1\ndone");

  std::cout << "Testing on input '" << input.str() << "'.\n";

  std::string msg = do_client(input);
  std::string expected("SORTED -12 3.6 30 500 88.1");
  std::cout << "Received server message '" << msg << "'.\n";

  if (msg != expected) {
    std::cout << "TEST FAILED. Expected '" << expected << "' but was '" << msg
              << "'.\n";
    return 1;
  }
  
  std::cout << "Recieved expected message '" << msg << "'. Test passed 😄\n";
  return 0;
}

std::string do_client(std::istream &in_stream) {
  /* TODO: ADD FUNCTION IMPLEMENTATION HERE */
  Socket sock(Socket::Family::INET, Socket::Type::STREAM);

  Address address("68.183.63.165");

  sock.Connect(address, 7778);

  printf("Connected!\n");

  std::stringstream message_stream;
  message_stream << "LIST";

  for (std::string line;;) {
    std::cout << "Enter a number: ";
    std::cout.flush();

    std::getline(in_stream, line);

    if (line == "done")
      break;

    if (line.empty() || isspace(line[0])) {
      std::cout << "Invalid input.\n";
    }

    char *end;
    long i = strtol(line.c_str(), &end, 10);
    if (end == line.c_str() || *end != '\0') {
      double d = strtod(line.c_str(), &end);
      if (end == line.c_str() || *end != '\0') {
        std::cout << "Invalid input.\n";
      } else {
        message_stream << " " << d;
      }
    } else {
      message_stream << " " << i;
    }
  }

  std::string to_send = message_stream.str();
  size_t len = sock.Send(to_send.data(), to_send.size());

  printf("Sent %d bytes\n", (int)len);

  static char recv_buffer[4096];

  size_t recv_buffer_len = sock.Recv(recv_buffer, sizeof(recv_buffer));

  std::cout << "Client received message '";
  std::cout.write(recv_buffer, recv_buffer_len);
  std::cout << "' of length " << recv_buffer_len << ".\n";

  return std::string(recv_buffer, recv_buffer_len);
}
