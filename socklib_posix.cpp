#include "socklib.h"
#include <cstring>
#include <memory>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sstream>
#include <stdexcept>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>
#include <string.h>

void SockLibInit() {}
void SockLibShutdown() {}

std::string to_string(const ByteString &s) {
  std::string str(s.begin(), s.end());
  return str;
}

union PosixAddress
{
  Address::AddressData generic_data;
  u_int32_t address;
};

static u_int32_t to_native_address(Address generic_address)
{
  PosixAddress posix_address;
  posix_address.generic_data = generic_address._data;
  return posix_address.address;
}

Address::Address(const std::string &name) {
  PosixAddress posix_address;

  u_int32_t address = 0;
  std::stringstream stream(name);
  std::string token;
  int offset = 24;
  for (int i = 0; i < 4; i++) {
    std::getline(stream, token, '.');
    unsigned int byte = 0;
    std::stringstream to_parse(token);
    to_parse >> byte;
    address += byte << (offset - (i * 8));
  }

  posix_address.address = htonl(address);
  _data = posix_address.generic_data;
}

union PosixSocket {
  Socket::SocketData generic_data;
  int posix_socket;
};

static int to_native_socket(const Socket& generic_socket)
{
  PosixSocket posix_socket;
  posix_socket.generic_data = generic_socket._data;
  return posix_socket.posix_socket;
}

// Socket Class

Socket::Socket() {
  memset(_data.data, 0, sizeof(_data.data));
  _has_socket = false;
}

Socket::Socket(Socket::Family family, Socket::Type type):Socket() {
  Create(family, type);
}

Socket::Socket(Socket&& other) {
  _has_socket = other._has_socket;
  memcpy(_data.data, other._data.data, sizeof(_data.data));

  other._has_socket = false;
  memset(other._data.data, 0, sizeof(other._data.data));
}

Socket::~Socket() {
  if (_has_socket) close(to_native_socket(*this));
}

void Socket::Create(Socket::Family family, Socket::Type type)
{
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
    native_protocol = 6;
    break;
  case Socket::Type::DGRAM:
    native_type = SOCK_DGRAM;
    native_protocol = 0; // I don't know this
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
  native_address.sin_addr.s_addr = to_native_address(address);

  if (bind(to_native_socket(*this), (sockaddr *)&native_address, sizeof(native_address)) ==
      -1) {
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
  native_address.sin_addr.s_addr = to_native_address(address);

  if (connect(to_native_socket(*this), (sockaddr *)&native_address, sizeof(native_address)) ==
      -1) {
    throw std::runtime_error(std::string("connect(): ") + strerror(errno));
  }

  return 0;
}

PoolView Socket::RecvIntoPool(unsigned int max_len) {
  PoolView pool = get_pool(max_len);
  pool.name = "Recv Temp Pool";

  Recv(*pool);

  return pool;
}

size_t Socket::Recv(ByteString &buffer) {
  buffer.resize(buffer.capacity());
  ssize_t len = recv(to_native_socket(*this), buffer.data(), buffer.capacity(), 0);
  if (len < 0) {
    throw std::runtime_error(std::string("recv(): ") + strerror(errno));
  }
  buffer.resize(len);

  return len;
}

size_t Socket::SendAll(const char *data, size_t len) {
  ssize_t send_count = 0;
  while (send_count < len) {
    ssize_t count =
      send(to_native_socket(*this), data + send_count, len - send_count, 0);
    if (count == -1) {
      throw std::runtime_error(std::string("send(): ") + strerror(errno));
    }
    send_count += count;
  }

  return send_count;
}

size_t Socket::SendAll(const ByteString &data) {
  return SendAll(data.data(), data.size());
}

ByteString to_bytestring(const char *msg, size_t len) {
  ByteString str;
  str.reserve(len);
  for (const char *p = msg; *p != '\0'; p++) {
    str.push_back(*p);
  }
  return str;
}

std::ostream &operator<<(std::ostream &s, const ByteString &b) {
  s.write(b.data(), b.size());
  return s;
}
