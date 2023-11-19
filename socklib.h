#include <memory>
#include <string>
#include <vector>

typedef std::string ByteString;

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
  size_t SendAll(const ByteString& data);

 private:
  class SocketData;
  SocketData* _data;
};

void SockLibInit();
void SockLibShutdown();
