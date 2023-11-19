#include "socklib.h"
#include <memory>
#include <sstream>
#include <vector>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdexcept>

class Address::AddressData
{
public:
  u_int32_t address;
};

Address::Address(const std::string& name)
{
  _data = new AddressData();

  u_int32_t address = 0;
  std::stringstream stream(name);
  std::string token;
  int offset = 24;
  for (int i = 0; i < 4; i++)
    {
      std::getline(stream, token, '.');
      unsigned int byte = 0;
      std::stringstream to_parse(token);
      to_parse >> byte;
      address += byte << (offset - (i * 8));
    }

  _data->address = htonl(address);
}

Address::~Address()
{
  delete _data;
}

class Socket::SocketData
{
public:
  int s;
};

// Socket Class

Socket::Socket(Socket::Family family, Socket::Type type)
{
  _data = new SocketData();
  int native_family;
  int native_type;
  int native_protocol;

  switch (family)
    {
    case Socket::Family::INET:
      native_family = AF_INET;
      break;
    case Socket::Family::INET6:
      native_family = AF_INET6;
      break;
    default:
      exit(1);
    }

  switch (type)
    {
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

  _data->s = socket(native_family, native_type, native_protocol);
  if (_data->s == -1)
    {
      perror("Opening socket");
      exit(1);
    }

}

Socket::~Socket()
{
  close(_data->s);
  delete _data;
}

int Socket::Bind(const Address& address, int port)
{
  sockaddr_in native_address;
  
  native_address.sin_family = AF_INET;
  native_address.sin_port = htons(port);
  native_address.sin_addr.s_addr = address._data->address;


  if (bind(_data->s, (sockaddr*)&native_address, sizeof(native_address)) == -1)
    {
      throw std::runtime_error(std::string("bind(): ") + strerror(errno));
    }
  
  return 0;
}

int Socket::Listen(int backlog)
{
  if (listen(_data->s, backlog) == -1)
    {
      throw std::runtime_error(std::string("listen(): ") + strerror(errno));
    }

  return 0;
}

std::unique_ptr<Socket> Socket::Accept()
{
  sockaddr conn_addr;
  socklen_t conn_addr_len;
  int connection = accept(_data->s, &conn_addr, &conn_addr_len);
  if (connection == -1)
    {
      throw std::runtime_error(std::string("accept(): ") + strerror(errno));
    }

  std::unique_ptr<Socket> conn_sock(new Socket(Socket::Family::INET, Socket::Type::STREAM));
  conn_sock->_data->s = connection;

  return conn_sock;
}

int Socket::Connect(const Address& address, int port)
{
  sockaddr_in native_address;
  
  native_address.sin_family = AF_INET;
  native_address.sin_port = htons(port);
  native_address.sin_addr.s_addr = address._data->address;

  if (connect(_data->s, (sockaddr*)&native_address, sizeof(native_address)) == -1)
    {
      throw std::runtime_error(std::string("connect(): ") + strerror(errno));
    }

  return 0;
}

ByteString Socket::Recv(unsigned int max_len)
{
  ByteString buffer;
  buffer.resize(max_len);

  size_t len = RecvInto(buffer);

  buffer.resize(len);
  
  return buffer;
}

size_t Socket::RecvInto(ByteString& buffer)
{
  ssize_t len = recv(_data->s, buffer.data(), buffer.size(), 0);
  if (len < 0)
    {
      throw std::runtime_error(std::string("recv(): ") + strerror(errno));
    }

  return len;
}

size_t Socket::SendAll(const ByteString& data)
{
  size_t send_count = 0;
  while (send_count < data.size())
    {
      ssize_t count = send(_data->s, data.data() + send_count, data.size() - send_count, 0);
      if (count == -1)
	{
	  throw std::runtime_error(std::string("send(): ") + strerror(errno));
	}
      send_count += count;
    }

  return send_count;
}
