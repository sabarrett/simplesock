#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <string>

#include "socklib.h"

int do_server(int sock);
int do_client(int sock);

void do_client(Socket& sock);
void do_server(Socket& sock);

int main(int argc, char* argv[])
{
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

int do_server(int sock)
{
  sockaddr_in address;
  address.sin_family = AF_INET;
  address.sin_port = htons(8000);
  address.sin_addr.s_addr = INADDR_ANY;
  
  if (bind(sock, (sockaddr*)&address, sizeof(address)) == -1)
    {
      perror("Binding socket");
      exit(1);
    }

  printf("Bound socket to port %d\n", ntohs(address.sin_port));

  if (listen(sock, 16) == -1)
    {
      perror("Listening");
      exit(1);
    }

  sockaddr conn_addr;
  socklen_t conn_addr_len;
  int connection = accept(sock, &conn_addr, &conn_addr_len);
  if (connection == -1)
    {
      perror("Accepting connection");
      exit(1);
    }

  printf("Recieved connection. New socket fd: %d", connection);

  char buffer[513] = {0};
  ssize_t len = recv(connection, buffer, 512, 0);
  if (len == -1)
    {
      throw std::runtime_error(std::string("recv():") + strerror(errno));
    }

  printf("Received message: '%s'", buffer);

  return 0;
}

int do_client(int sock)
{
  sockaddr_in address;
  address.sin_family = AF_INET;
  address.sin_port = htons(8000);
  address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

  printf("Attempting to connect...\n");
  
  if (connect(sock, (sockaddr*)&address, sizeof(address)) == -1)
    {
      throw std::runtime_error(std::string("connect():") + strerror(errno));
    }

  printf("Connected!");

  std::string msg = "Hi, server!";
  ssize_t send_count = 0;
  while (send_count < msg.length())
    {
      ssize_t count = send(sock, msg.c_str(), msg.length(), 0);
      if (count == -1)
	{
	  throw std::runtime_error(std::string("send():") + strerror(errno));
	}
      send_count += count;
    }

  printf("Sent %ld bytes.", send_count);
  
  return 0;
}

void do_client(Socket& sock)
{
  Address address("127.0.0.1");

  sock.Connect(address, 8000);

  printf("Connected!\n");

  ssize_t len = sock.SendAll("Hi, server!");

  printf("Sent %ld bytes\n", len);

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

  printf("Received message of length %ld: '%s'\n", msg.length(), msg.c_str());

  connection->SendAll("Hi there, client!");
}
