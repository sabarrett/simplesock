#include "socklib.h"
#include <arpa/inet.h>
#include <cmath>
#include <cstring>
#include <cassert>
#include <memory>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sstream>
#include <stdexcept>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>
#include <fcntl.h>

void SockLibInit() {}
void SockLibShutdown() {}

union PosixAddress {
  Address::AddressData generic_data;
  sockaddr_in address;
};

static sockaddr_in to_native_address(Address generic_address) {
  PosixAddress posix_address;
  posix_address.generic_data = generic_address._data;
  return posix_address.address;
}

Address::Address(const std::string &name, int port) {
  PosixAddress posix_address;
  int result;
  if ((result = inet_pton(AF_INET, name.c_str(), &posix_address.address.sin_addr)) != 1) {
    if (result == 0) {
      throw std::runtime_error(
          std::string("Failed to parse IP address '" + name + "'\n"));
    }

    throw std::runtime_error(std::string("inet_pton(): ") + strerror(errno));
  }

  posix_address.address.sin_port = htons(port);
  posix_address.address.sin_family = AF_INET;

  assert(sizeof(_data) >= sizeof(posix_address.address));
  memcpy(&_data, &posix_address, sizeof(posix_address));
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

int Socket::SetNonBlockingMode(bool shouldBeNonBlocking) {
  if (!_has_socket) {
    throw std::runtime_error(std::string("Socket has not yet been created"));
  }

  int sock = to_native_socket(*this);

  int flags = fcntl(sock, F_GETFL, 0);
  if (flags == -1) {
    throw std::runtime_error(std::string("fcntl(): ") + strerror(errno));
  }
  if (shouldBeNonBlocking) {
    flags |= O_NONBLOCK;
  } else {
    flags &= ~O_NONBLOCK;
  }

  int result = fcntl(sock, F_SETFL, flags);
  if (result == -1) {
    throw std::runtime_error(std::string("fcntl(): ") + strerror(errno));
  }

  return 0;
}

int Socket::SetTimeout(float seconds) {
  float i, f;
  f = modff(seconds, &i);
  timeval tv = {0};
  tv.tv_sec = (int)i;
  tv.tv_usec = f * (int)1e6;
  int result = setsockopt(to_native_socket(*this),
			  SOL_SOCKET, SO_RCVTIMEO,
			  &tv, sizeof(tv));
  if (result == -1) 
    throw std::runtime_error(std::string("setsockopt():") + strerror(errno));

  return result;
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
    break;
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

int Socket::Bind(const Address &address) {
  sockaddr_in native_addr = to_native_address(address);

  if (bind(to_native_socket(*this), (sockaddr *)&native_addr,
           sizeof(native_addr)) == -1) {
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

int Socket::Connect(const Address &address) {
  sockaddr_in native_addr = to_native_address(address);

  if (connect(to_native_socket(*this), (sockaddr *)&native_addr,
              sizeof(native_addr)) == -1) {
    throw std::runtime_error(std::string("connect(): ") + strerror(errno));
  }

  return 0;
}

int Socket::Recv(char *buffer, int size) {
  ssize_t len = recv(to_native_socket(*this), buffer, size, 0);
  if (len == -1) {
    if (errno == EAGAIN) {
      _last_error = SOCKLIB_ETIMEDOUT;
      return -1;
    }
    throw std::runtime_error(std::string("recv(): ") + strerror(errno));
  }

  return (int)len;
}

int Socket::RecvFrom(char* buffer, int size, Address& src) {
  PosixAddress native_addr;
  socklen_t socklen = sizeof(native_addr.address);
  ssize_t count = recvfrom(to_native_socket(*this), buffer, size, 0, (sockaddr*)&native_addr.address, &socklen);
  if (count == -1) {
    if (errno == EAGAIN) {
      _last_error = SOCKLIB_ETIMEDOUT;
      return -1;
    }
    throw std::runtime_error(std::string("recvfrom(): ") + strerror(errno));
  }

  src._data = native_addr.generic_data;
  return count;
}

size_t Socket::Send(const char *data, size_t len) {
  ssize_t count = send(to_native_socket(*this), data, len, 0);
  if (count == -1) {
    throw std::runtime_error(std::string("send(): ") + strerror(errno));
  }

  return count;
}

size_t Socket::SendTo(const char* data, size_t len, const Address& dst) {
  sockaddr_in native_addr = to_native_address(dst);

  ssize_t count = sendto(to_native_socket(*this), data, len, 0, (sockaddr*)&native_addr, sizeof(native_addr));
  if (count == -1) {
    throw std::runtime_error(std::string("sendto(): ") + strerror(errno));
  }

  return count;
}

std::ostream& operator<<(std::ostream& s, const Address& a) {
  sockaddr_in nat_addr = to_native_address(a);
  s << inet_ntoa(nat_addr.sin_addr);
  s << ":";
  s << ntohs(nat_addr.sin_port);

  return s;
}
