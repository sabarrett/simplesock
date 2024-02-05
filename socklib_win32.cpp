#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN

#include "socklib.h"
#include <WS2tcpip.h>
#include <Windows.h>
#include <winsock2.h>

#include <iostream>
#include <stdexcept>

#pragma comment(lib, "Ws2_32.lib")

#define fatal(ctx)                                                             \
  {                                                                            \
    std::cerr << __FILE__ << " L" << __LINE__ << ": " << ctx << ": "           \
              << std::system_category().message(WSAGetLastError());            \
    abort();                                                                   \
  }

#define require(condition, ctx)                                                \
  if (!(condition)) {                                                          \
    fatal(ctx)                                                                 \
  };

static bool winsock_initialized = false;

void SockLibInit() {
  if (!winsock_initialized) {
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != NO_ERROR) {
      throw std::runtime_error(std::string("WSAStartup(): ") +
                               std::to_string(iResult));
    }
    winsock_initialized = true;
  }
}

void SockLibShutdown() {
  if (winsock_initialized) {
    WSACleanup();
  }
}

union Win32Address {
  Address::AddressData generic_data;
  IN_ADDR address;
};

static IN_ADDR to_native_address(Address generic_address) {
  Win32Address win32_address;
  win32_address.generic_data = generic_address._data;
  return win32_address.address;
}

Address::Address(const std::string &name) {
  require(inet_pton(AF_INET, name.c_str(), &_data) == 1, "inet_pton");
}

union Win32Socket {
  Socket::SocketData generic_data;
  SOCKET s;
};

static SOCKET to_native_socket(const Socket &generic_socket) {
  Win32Socket win32_socket;
  win32_socket.generic_data = generic_socket._data;
  return win32_socket.s;
}

void Socket::native_destroy(Socket &socket) {
  closesocket(to_native_socket(socket));
}

void Socket::Create(Socket::Family family, Socket::Type type) {
  if (_has_socket)
    throw std::runtime_error("Socket already has an associated system socket.");

  int native_family;
  int native_type;
  int native_protocol;

  switch (family) {
  case INET:
    native_family = AF_INET;
    break;
  default:
    throw std::runtime_error("Not implemented");
  }

  switch (type) {
  case STREAM:
    native_type = SOCK_STREAM;
    native_protocol = IPPROTO_TCP;
    break;
  default:
    throw std::runtime_error("Not implemented");
  }

  Win32Socket sock;

  sock.s = socket(native_family, native_type, native_protocol);
  require(sock.s != INVALID_SOCKET, "socket()");
  _data = sock.generic_data;

  _has_socket = true;
}

int Socket::Bind(const Address &address, int port) {
  sockaddr_in service;
  service.sin_family = AF_INET;
  service.sin_port = htons(port);
  service.sin_addr = to_native_address(address);

  require(bind(to_native_socket(*this), (sockaddr *)&service,
               sizeof(service)) != SOCKET_ERROR,
          "bind()");

  return 0;
}

int Socket::Listen(int backlog) {
  require(listen(to_native_socket(*this), backlog) != SOCKET_ERROR, "listen()");

  return 0;
}

Socket Socket::Accept() {
  SOCKET connection = accept(to_native_socket(*this), NULL, NULL);
  require(connection != INVALID_SOCKET, "accept()");

  Socket conn_sock(Socket::Family::INET, Socket::Type::STREAM);
  Win32Socket sock;
  sock.s = connection;
  conn_sock._data = sock.generic_data;

  return conn_sock;
}

int Socket::Connect(const Address &address, int port) {
  sockaddr_in service;
  service.sin_family = AF_INET;
  service.sin_port = htons(port);
  service.sin_addr = to_native_address(address);

  require(connect(to_native_socket(*this), (sockaddr *)&service,
                  sizeof(service)) != SOCKET_ERROR,
          "connect()");

  return 0;
}

size_t Socket::Recv(char *buffer, size_t size) {
  int len = recv(to_native_socket(*this), buffer, (int)size, 0);
  require(len >= 0, "recv()");

  return (size_t)len;
}

size_t Socket::Send(const char *data, size_t len) {
    int len_i = (int)len;
  int count = send(to_native_socket(*this), data, len_i, 0);
  if (count == SOCKET_ERROR) {
    throw std::runtime_error(std::string("send(): ") + strerror(errno));
  }
  return count;
}
