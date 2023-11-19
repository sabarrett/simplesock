#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <functional>

#include "socklib.h"

void do_client(Socket& sock);
void do_server(Socket& sock);

class defer
{
public:
    defer(std::function<void()> fn) : fn(fn) {}
    ~defer() { fn(); }

private:
    std::function<void()> fn;
};

int main(int argc, char* argv[])
{
    SockLibInit();
    defer _shutdown_socklib([]() { SockLibShutdown(); });

    Socket sock(Socket::Family::INET, Socket::Type::STREAM);

    if (argc > 1)
    {
        do_server(sock);
    }
    else
    {
        do_client(sock);
    }
}


void do_client(Socket& sock)
{
  Address address("127.0.0.1");

  sock.Connect(address, 8000);

  printf("Connected!\n");

  size_t len = sock.SendAll("Hi, server!");

  printf("Sent %d bytes\n", (int)len);

  std::string server_msg = sock.Recv(1024);

  std::cout << "Client received message '" << server_msg << "' of length " << server_msg.length() << ".\n";
}

void do_server(Socket& sock)
{
  Address address("0.0.0.0");

  sock.Bind(address, 8000);

  printf("Bound socket successfully\n");

  sock.Listen();

  printf("Listened successfully\n");

  std::shared_ptr<Socket> connection = sock.Accept();

  printf("Accepted connection!\n");

  std::string msg = connection->Recv(1024);

  printf("Received message of length %d: '%s'\n", (int)msg.length(), msg.c_str());

  connection->SendAll("Hi there, client!");
}


