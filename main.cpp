#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>

#include "defer.h"
#include "socklib.h"

void do_client(Socket& sock);
void do_server(Socket& sock);
void print_allocations(bool to_print);

int main(int argc, char* argv[])
{
    SockLibInit();
    defer _shutdown_socklib([]() { SockLibShutdown(); });

    Socket sock(Socket::Family::INET, Socket::Type::STREAM);

    std::vector<size_t> pool_sizes;
    pool_sizes.reserve(8);
    pool_sizes.push_back(512);
    pool_sizes.push_back(512);
    pool_sizes.push_back(2048);
    pool_sizes.push_back(4096);

    init_pools(pool_sizes);

    print_allocations(true);

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

  PoolView view = get_pool(4);
  view.name = "Client send stream pool";
  OutDataStream stream(view.vector());
  stream.PutUInt32(88223399);

  //ssize_t len = sock.SendAll("Hi, server!");
  ssize_t len = sock.SendAll(stream.GetByteString());

  printf("Sent %d bytes\n", (int)len);

  PoolView recvPool = sock.Recv(1024);
  recvPool.name = "Client recv pool";

  std::cout << "Client received message '" << recvPool.vector() << "' of length " << recvPool.vector().size() << ".\n";
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

  PoolView view = connection->Recv(1024);
  view.name = "Server recv pool";
  
  InDataStream stream(view.vector());

  ByteString& msg = stream.buffer();

  std::cout << "Received message of length " << msg.size() << ": '" << msg << std::endl;

  // printf("Received message of length %d: '%*s'\n", (int)msg.size(), (int)msg.size(), msg.data());

  // InDataStream stream(std::move(msg));
  int x = stream.ReadUInt32();

  std::cout << "x = " << x << std::endl;

  connection->SendAll(STR_ARGS("Hi there, client!"));
}

