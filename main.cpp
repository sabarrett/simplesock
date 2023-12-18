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

  OutDataStream stream(4);
  stream.PutUInt32(88223399);

  //ssize_t len = sock.SendAll("Hi, server!");
  ssize_t len = sock.SendAll(stream.GetByteString());

  printf("Sent %d bytes\n", (int)len);

  ByteString server_msg = sock.Recv(1024);

  std::cout << "Client received message '" << server_msg << "' of length " << server_msg.size() << ".\n";
}

#define STR_ARGS(x) x, sizeof(x)

void do_server(Socket& sock)
{
  Address address("0.0.0.0");

  sock.Bind(address, 8000);

  printf("Bound socket successfully\n");

  sock.Listen();

  printf("Listened successfully\n");

  std::unique_ptr<Socket> connection = sock.Accept();

  printf("Accepted connection!\n");

  // ByteString msg = connection->Recv(1024);

  InDataStream stream(1024);

  connection->RecvInto(stream.buffer());

  ByteString& msg = stream.buffer();

  std::cout << "Received message of length " << msg.size() << ": '" << msg << std::endl;

  // printf("Received message of length %d: '%*s'\n", (int)msg.size(), (int)msg.size(), msg.data());

  // InDataStream stream(std::move(msg));
  int x = stream.ReadUInt32();

  std::cout << "x = " << x << std::endl;

  connection->SendAll(STR_ARGS("Hi there, client!"));
}

