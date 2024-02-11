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
  SOCKADDR_IN address;
};

static SOCKADDR_IN to_native_address(Address generic_address) {
  Win32Address win32_address;
  win32_address.generic_data = generic_address._data;
  return win32_address.address;
}

Address::Address(const std::string &name, int port) {
  Win32Address win32_addr;
  require(inet_pton(AF_INET, name.c_str(), &win32_addr.address.sin_addr) == 1, "inet_pton");
  win32_addr.address.sin_port = htons(port);
  win32_addr.address.sin_family = AF_INET;

  memcpy(&_data, &win32_addr, sizeof(win32_addr));
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

int Socket::SetNonBlockingMode(bool shouldBeNonBlocking) {
  if (!_has_socket) {
    throw std::runtime_error(std::string("Socket has not yet been created"));
  }

  SOCKET sock = to_native_socket(*this);
  unsigned long value = shouldBeNonBlocking ? 1 : 0;
  int result = ioctlsocket(sock, FIONBIO, &value);
  require(result != SOCKET_ERROR, "ioctlsocket()");

  return 0;
}

int Socket::SetTimeout(float seconds) {
  DWORD value = (DWORD)(seconds * 1000);
  int result = setsockopt(to_native_socket(*this),
			  SOL_SOCKET, SO_RCVTIMEO,
			  (const char*)&value, sizeof(value));
  require(result == 0, "setsockopt()");

  return 0;
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
  case DGRAM:
    native_type = SOCK_DGRAM;
    native_protocol = IPPROTO_UDP;
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

int Socket::Bind(const Address &address) {
  SOCKADDR_IN native_addr = to_native_address(address);
  require(bind(to_native_socket(*this), (sockaddr *)&native_addr,
               sizeof(native_addr)) != SOCKET_ERROR,
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

int Socket::Connect(const Address &address) {
  SOCKADDR_IN native_addr = to_native_address(address);

  require(connect(to_native_socket(*this), (sockaddr *)&native_addr,
                  sizeof(native_addr)) != SOCKET_ERROR,
          "connect()");

  return 0;
}

int Socket::Recv(char *buffer, int size) {
  int len = recv(to_native_socket(*this), buffer, size, 0);
  if (len == SOCKET_ERROR) {
    if (WSAGetLastError() == WSAETIMEDOUT) {
      _last_error = SOCKLIB_ETIMEDOUT;
      return -1;
    }
  }
  // Crash on all other errors
  require(len >= 0, "recv()");

  return len;
}

int Socket::RecvFrom(char* buffer, int size, Address& src) {
  Win32Address native_addr;
  int socklen = sizeof(native_addr.address);
  int count = recvfrom(to_native_socket(*this), buffer, size, 0, (sockaddr*)&native_addr.address, &socklen);
  if (count == SOCKET_ERROR) {
    if (WSAGetLastError() == WSAETIMEDOUT) {
      _last_error = SOCKLIB_ETIMEDOUT;
      return -1;
    }
  }

  // Crash on all other errors
  require(count != SOCKET_ERROR, "recvfrom()");

  src._data = native_addr.generic_data;
  return count;
}

size_t Socket::Send(const char *data, size_t len) {
    int len_i = (int)len;
  int count = send(to_native_socket(*this), data, len_i, 0);
  if (count == SOCKET_ERROR) {
    throw std::runtime_error(std::string("send(): ") + strerror(errno));
  }
  return count;
}

size_t Socket::SendTo(const char* data, size_t len, const Address& dst) {
  SOCKADDR_IN native_addr = to_native_address(dst);

  int count = sendto(to_native_socket(*this), data, (int)len, 0, (sockaddr*)&native_addr, sizeof(native_addr));
  require(count != SOCKET_ERROR, "sendto()");

  return count;
}

std::ostream& operator<<(std::ostream& s, const Address& a) {
  SOCKADDR_IN nat_addr = to_native_address(a);
  s << inet_ntoa(nat_addr.sin_addr);
  s << ":";
  s << ntohs(nat_addr.sin_port);

  return s;
}
