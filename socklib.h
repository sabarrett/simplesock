#include <memory>
#include <string>
#include <vector>
#include <iostream>
#include "pool.h"

typedef std::vector<char> ByteString;

class Address
{
 public:
  Address(const std::string& name);

  struct AddressData
  {
    char data[32];
  } _data;
};

class Socket
{
 public:
  enum Family
  {
    INET,
    INET6
  };

  enum Type
  {
    STREAM,
    DGRAM
  };

  Socket();
  Socket(Family family, Type type);
  ~Socket();

  Socket(const Socket& other) = delete;
  Socket(Socket&& other);

  void Create(Family family, Type type);
  int Bind(const Address& address, int port);
  int Listen(int backlog=16);
  Socket Accept();
  int Connect(const Address& address, int port);
  PoolView RecvIntoPool(unsigned int max_len);
  size_t Recv(char* buffer, size_t size);
  size_t Recv(ByteString& buffer);
  size_t Send(const char* data, size_t len);
  size_t SendAll(const char* data, size_t len);
  size_t SendAll(const ByteString& data);

  static void native_destroy(Socket& socket);

  bool _has_socket;

  struct SocketData
  {
    char data[32];
  } _data;
};

void SockLibInit();
void SockLibShutdown();

ByteString to_bytestring(const char* msg, size_t len);
std::ostream& operator<<(std::ostream& s, const ByteString& b);
