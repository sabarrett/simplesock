#include <cstdio>
#include <iostream>
#include <ostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sstream>

#include <string.h>

#include "defer.h"
#include "socklib.h"
#include "allocators.h"

std::string do_client(std::istream& in_stream);
// void do_server(Socket &sock);

int main(int argc, char *argv[]) {
    
    SockLibInit();
    defer _shutdown_socklib([]() { SockLibShutdown(); });
    
    std::stringstream input("30\n500\n-12\n3.6\nnot a number\n200also not a number\n88.1\ndone");

    std::string msg = do_client(input);
    std::string expected("SORTED -12 3.6 30 500 88.1");
    if (msg != expected)
    {
	std::cout << "TEST FAILED. Expected '" << expected << "' but was '" << msg << "'.\n";
	exit(1);
    }
    else
    {
	std::cout << "Test passed 😄\n";
    }

    return 0;
}

#define STR_ARGS(x) x, sizeof(x) - 1

ByteString& bytestring_append(ByteString &str, const char *c_str) {
  while (*c_str != '\0') {
    str.push_back(*c_str++);
  }
  return str;
}

std::string do_client(std::istream& in_stream) {
    Socket sock(Socket::Family::INET, Socket::Type::STREAM);

  Address address("68.183.63.165");

  sock.Connect(address, 7778);

  printf("Connected!\n");

  std::stringstream message_stream;
  message_stream << "LIST";

  for (std::string line;;)
  {
      std::cout << "Enter a number: ";
      std::cout.flush();

      std::getline(in_stream, line);

      if (line == "done")
	  break;

      if (line.empty() || isspace(line[0]))
      {
	  std::cout << "Invalid input.\n";
      }
	  
      char* end;
      long i = strtol(line.c_str(), &end, 10);
      if (end == line.c_str() || *end != '\0')
      {
	  double d = strtod(line.c_str(), &end);
	  if (end == line.c_str() || *end != '\0')
	  {
	      std::cout << "Invalid input.\n";
	  }
	  else
	  {
	      message_stream << " " << d;
	  }
      }
      else
      {
	  message_stream << " " << i;
      }
  }

  std::string to_send = message_stream.str();
  size_t len = sock.Send(to_send.data(), to_send.size());

  printf("Sent %d bytes\n", (int)len);

  static char recv_buffer[1024];

  size_t recv_buffer_len = sock.Recv(recv_buffer, sizeof(recv_buffer));

  std::cout << "Client received message '";
  std::cout.write(recv_buffer, recv_buffer_len);
  std::cout << "' of length " << recv_buffer_len << ".\n";

  return std::string(recv_buffer, recv_buffer_len);
}

void do_server(Socket &sock) {
  Address address("0.0.0.0");

  sock.Bind(address, 8000);

  printf("Bound socket successfully\n");

  sock.Listen();

  printf("Listened successfully\n");

  Socket connection = sock.Accept();

  printf("Accepted connection!\n");

  ByteString msg;
  msg.resize(1024);

  connection.Recv(msg);

  std::cout << "Received message of length " << msg.size() << ": '" << msg
            << "'\n";

  connection.SendAll(STR_ARGS("Hi there, client!"));
}
