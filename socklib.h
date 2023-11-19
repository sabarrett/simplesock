#include <memory>
#include <string>
#include <vector>

typedef std::string ByteString;

/*
class ByteString : public std::vector<char>
{
 public:
  explicit ByteString(size_type n, const value_type& val, const allocator_type& alloc = allocator_type()): std::vector<char>(n, val, alloc) {}
  std::string as_ascii();
};
*/

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
  std::shared_ptr<Socket> Accept();
  int Connect(const Address& address, int port);
  ByteString Recv(unsigned int max_len);
  size_t RecvInto(ByteString& buffer);
  ssize_t SendAll(const ByteString& data);

 private:
  class SocketData;
  SocketData* _data;
};

