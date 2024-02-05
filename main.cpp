#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "defer.h"
#include "socklib.h"

void do_client(Socket &sock);
void do_server(Socket &sock);

int main(int argc, char *argv[]) {
  SockLibInit();
  defer _shutdown_socklib([]() { SockLibShutdown(); });

  Socket sock(Socket::Family::INET, Socket::Type::STREAM);

  if (argc > 1) {
    do_server(sock);
  } else {
    do_client(sock);
  }
}

#define STR_ARGS(x) x, sizeof(x) - 1

ByteString& bytestring_append(ByteString &str, const char *c_str) {
  while (*c_str != '\0') {
    str.push_back(*c_str++);
  }
  return str;
}

void do_client(Socket &sock) {
  Address address("68.183.63.165");

  sock.Connect(address, 7778);

  printf("Connected!\n");
  char to_send[] = "LIST -9 7 401 100";
  size_t len = sock.Send(to_send, sizeof(to_send) - 1);

  printf("Sent %d bytes\n", (int)len);

  static char recv_buffer[1024];

  size_t recv_buffer_len = sock.Recv(recv_buffer, sizeof(recv_buffer));

  std::cout << "Client received message '";
  std::cout.write(recv_buffer, recv_buffer_len);
  std::cout << "' of length " << recv_buffer_len << ".\n";
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
