#include <memory>
#include <string>
#include <vector>
#include <iostream>

typedef std::vector<char> ByteString;

class Address
{
 public:
  Address(const std::string& name);
  ~Address();

  class AddressData;
  AddressData* _data;
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

  Socket(Family family, Type type);
  ~Socket();

  int Bind(const Address& address, int port);
  int Listen(int backlog=16);
  std::unique_ptr<Socket> Accept();
  int Connect(const Address& address, int port);
  ByteString Recv(unsigned int max_len);
  size_t RecvInto(ByteString& buffer);
  size_t SendAll(const char* data);
  size_t SendAll(const ByteString& data);

 private:
  class SocketData;
  SocketData* _data;
};

void SockLibInit();
void SockLibShutdown();

ByteString to_bytestring(const char* msg);
std::ostream& operator<<(std::ostream& s, const ByteString& b);

class InDataStream
{
 public:
 InDataStream(ByteString string):
  string(std::move(string)),
    head(0)
    {}

  int ReadUInt32()
  {
    uint32_t x;

    x = *(uint32_t*)&string[head];
    x = ntohl(x);

    return x;
  }
  
 private:
  const ByteString string;
  int head;
};

class OutDataStream
{
 public:

 OutDataStream():
  string()
    {}
  
 OutDataStream(size_t max_size):
  string(max_size)
    {}

  ByteString& GetByteString() {return string;}

  void PutUInt32(uint32_t x)
  {
    Data_32 data;
    data.as_uint = htonl(x);
    for (int i = 0; i < 4; i++)
      {
	string.push_back(data.c[i]);
      }
  }

 private:
  union Data_32
  {
    char c[4];
    uint32_t as_uint;
    int32_t as_int;
  };

  ByteString string;
};
