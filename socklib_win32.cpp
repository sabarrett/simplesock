#include "socklib.h"

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <Windows.h>
#include <WS2tcpip.h>

#include <stdexcept>
#include <iostream>

#pragma comment(lib, "Ws2_32.lib")

#define fatal(ctx) {std::cerr << __FILE__ << " L" << __LINE__ << ": " << ctx << ": " << std::system_category().message(WSAGetLastError()); abort();}

#define require(condition, ctx) if (!(condition)) {fatal(ctx)};

static bool winsock_initialized = false;

void SockLibInit()
{
	if (!winsock_initialized)
	{
		WSADATA wsaData;
		int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (iResult != NO_ERROR) {
			throw std::runtime_error(std::string("WSAStartup(): ") + std::to_string(iResult));
		}
		winsock_initialized = true;
	}
}

void SockLibShutdown()
{
	if (winsock_initialized)
	{
		WSACleanup();
	}
}

class Address::AddressData
{
public:
	IN_ADDR address;
};

Address::Address(const std::string& name)
{
	_data = new AddressData();
	require(inet_pton(AF_INET, name.c_str(), &_data->address) == 1, "inet_pton");
}

Address::~Address()
{
	delete _data;
}

class Socket::SocketData
{
public:
	SOCKET s;
};

Socket::Socket(Family family, Type type)
{
	_data = new SocketData();

	int native_family;
	int native_type;
	int native_protocol;

	switch (family)
	{
	case INET:
		native_family = AF_INET;
		break;
	default:
		throw std::runtime_error("Not implemented");
	}

	switch (type)
	{
	case STREAM:
		native_type = SOCK_STREAM;
		native_protocol = IPPROTO_TCP;
		break;
	default:
		throw std::runtime_error("Not implemented");
	}

	_data->s = socket(native_family, native_type, native_protocol);
	require(_data->s != INVALID_SOCKET, "socket()");
}

Socket::~Socket()
{
	closesocket(_data->s);
	delete _data;
}

int Socket::Bind(const Address& address, int port)
{
	sockaddr_in service;
	service.sin_family = AF_INET;
	service.sin_port = htons(port);
	service.sin_addr = address._data->address;

	require(bind(_data->s, (sockaddr*)&service, sizeof(service)) != SOCKET_ERROR, "bind()");

	return 0;
}

int Socket::Listen(int backlog)
{
	require(listen(_data->s, backlog) != SOCKET_ERROR, "listen()");

	return 0;
}

std::unique_ptr<Socket> Socket::Accept()
{
	SOCKET connection = accept(_data->s, NULL, NULL);
	require(connection != INVALID_SOCKET, "accept()");

	std::unique_ptr<Socket> s(new Socket(INET, STREAM));
	s->_data->s = connection;

	return s;
}

int Socket::Connect(const Address& address, int port)
{
	sockaddr_in service;
	service.sin_family = AF_INET;
	service.sin_port = htons(port);
	service.sin_addr = address._data->address;

	require(connect(_data->s, (sockaddr*)&service, sizeof(service)) != SOCKET_ERROR, "connect()");

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
	int len = recv(_data->s, buffer.data(), buffer.size(), 0);
	require(len >= 0, "recv()");

	return len;
}

size_t Socket::SendAll(const ByteString& data)
{
	size_t send_count = 0;
	while (send_count < data.size())
	{
		int count = send(_data->s, data.data() + send_count, data.size() - send_count, 0);
		require(count != -1, "send()");
		send_count += count;
	}

	return send_count;
}