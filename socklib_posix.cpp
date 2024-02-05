#include "socklib.h"
#include <arpa/inet.h>
#include <cstring>
#include <memory>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sstream>
#include <stdexcept>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

void SockLibInit() {}
void SockLibShutdown() {}

union PosixAddress {
  Address::AddressData generic_data;
  in_addr address;
};

static in_addr to_native_address(Address generic_address) {
  PosixAddress posix_address;
  posix_address.generic_data = generic_address._data;
  return posix_address.address;
}

Address::Address(const std::string &name) {
  PosixAddress posix_address;
  int result;
  if ((result = inet_pton(AF_INET, name.c_str(), &_data)) != 1) {
    if (result == 0) {
      throw std::runtime_error(
          std::string("Failed to parse IP address '" + name + "'\n"));
    }

    throw std::runtime_error(std::string("inet_pton(): ") + strerror(errno));
  }
}

union PosixSocket {
  Socket::SocketData generic_data;
  int posix_socket;
};

static int to_native_socket(const Socket &generic_socket) {
  PosixSocket posix_socket;
  posix_socket.generic_data = generic_socket._data;
  return posix_socket.posix_socket;
}

// Socket Class

void Socket::native_destroy(Socket& socket) {
    close(to_native_socket(socket));
}


void Socket::Create(Socket::Family family, Socket::Type type) {
  if (_has_socket)
    throw std::runtime_error("Socket already has an associated system socket.");

  int native_family;
  int native_type;
  int native_protocol;

  switch (family) {
  case Socket::Family::INET:
    native_family = AF_INET;
    break;
  case Socket::Family::INET6:
    native_family = AF_INET6;
    break;
  default:
    exit(1);
  }

  switch (type) {
  case Socket::Type::STREAM:
    native_type = SOCK_STREAM;
    native_protocol = IPPROTO_TCP;
    break;
  case Socket::Type::DGRAM:
    native_type = SOCK_DGRAM;
    native_protocol = IPPROTO_UDP;
  default:
    exit(1);
  }

  PosixSocket sock;

  sock.posix_socket = socket(native_family, native_type, native_protocol);
  if (sock.posix_socket == -1) {
    throw std::runtime_error(std::string("socket(): ") + strerror(errno));
  }
  _data = sock.generic_data;

  _has_socket = true;
}

int Socket::Bind(const Address &address, int port) {
  sockaddr_in native_address;

  native_address.sin_family = AF_INET;
  native_address.sin_port = htons(port);
  native_address.sin_addr = to_native_address(address);

  if (bind(to_native_socket(*this), (sockaddr *)&native_address,
           sizeof(native_address)) == -1) {
    throw std::runtime_error(std::string("bind(): ") + strerror(errno));
  }

  return 0;
}

int Socket::Listen(int backlog) {
  if (listen(to_native_socket(*this), backlog) == -1) {
    throw std::runtime_error(std::string("listen(): ") + strerror(errno));
  }

  return 0;
}

Socket Socket::Accept() {
  sockaddr conn_addr;
  socklen_t conn_addr_len;
  int connection = accept(to_native_socket(*this), &conn_addr, &conn_addr_len);
  if (connection == -1) {
    throw std::runtime_error(std::string("accept(): ") + strerror(errno));
  }

  Socket conn_sock(Socket::Family::INET, Socket::Type::STREAM);
  PosixSocket sock;
  sock.posix_socket = connection;
  conn_sock._data = sock.generic_data;

  return conn_sock;
}

int Socket::Connect(const Address &address, int port) {
  sockaddr_in native_address;

  native_address.sin_family = AF_INET;
  native_address.sin_port = htons(port);
  native_address.sin_addr = to_native_address(address);

  if (connect(to_native_socket(*this), (sockaddr *)&native_address,
              sizeof(native_address)) == -1) {
    throw std::runtime_error(std::string("connect(): ") + strerror(errno));
  }

  return 0;
}

size_t Socket::Recv(char *buffer, size_t size) {
  ssize_t len = recv(to_native_socket(*this), buffer, size, 0);
  if (len < 0) {
    throw std::runtime_error(std::string("recv(): ") + strerror(errno));
  }

  return len;
}

size_t Socket::Send(const char *data, size_t len) {
  ssize_t count = send(to_native_socket(*this), data, len, 0);
  if (count == -1) {
    throw std::runtime_error(std::string("send(): ") + strerror(errno));
  }

  return count;
}
