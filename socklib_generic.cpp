#include <iostream>
#include "socklib.h"

std::string to_string(const ByteString &s) {
  std::string str(s.begin(), s.end());
  return str;
}

Socket::Socket() {
  memset(_data.data, 0, sizeof(_data.data));
  _has_socket = false;
}

Socket::Socket(Socket::Family family, Socket::Type type) : Socket() {
  Create(family, type);
}

Socket::Socket(Socket &&other) {
  _has_socket = other._has_socket;
  memcpy(_data.data, other._data.data, sizeof(_data.data));

  other._has_socket = false;
  memset(other._data.data, 0, sizeof(other._data.data));
}

Socket::~Socket() {
  if (_has_socket) native_destroy(*this);
}

size_t Socket::SendAll(const ByteString &data) {
    return SendAll(data.data(), data.size());
}

size_t Socket::SendAll(const char *data, size_t len) {
  size_t send_count = 0;
  while (send_count < len) {
    size_t count = Send(data + send_count, len - send_count);
    send_count += count;
  }

  return send_count;
}

size_t Socket::Recv(ByteString &buffer) {
    return Recv(buffer.data(), buffer.size());
}

PoolView Socket::RecvIntoPool(unsigned int max_len) {
  PoolView pool = get_pool(max_len);
  pool.name = "Recv Temp Pool";

  Recv(*pool);

  return pool;
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
