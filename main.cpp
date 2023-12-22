#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <functional>

#include "socklib.h"

void do_client(Socket& sock);
void do_server(Socket& sock);
void print_allocations(bool to_print);

struct Pool
{
    std::vector<char> pool;
    int lock;
};

class PoolView
{
public:
    PoolView(Pool& pool):
	pool(pool)
    {
	pool.lock++;
    }

    ~PoolView()
    {
	pool.lock--;
    }

    Pool& pool;
    std::vector<char>& vector() {return pool.pool;}
};

std::vector<Pool> pools;

void add_pool_of_size(size_t size)
{
    Pool p;
    pools.push_back(p);
    Pool& pool = pools.back();
    pool.pool.reserve(size);
    pool.lock = 0;
}

void init_pools(std::vector<size_t> sizes)
{
    pools.reserve(sizes.size());
    for (size_t size : sizes)
    {
	add_pool_of_size(size);
    }
}

PoolView get_pool(size_t min_size)
{
    for (Pool& pool : pools)
    {
	if (pool.lock == 0 && pool.pool.capacity() >= min_size)
	{
	    pool.pool.clear();
	    return PoolView(pool);
	}
    }

    // No pools large enough. Create a new one with size of the next
    // power of two.
    float fp2 = log2(min_size);
    size_t target_size = 1 << (int)ceil(fp2);
    printf("Pools exhausted! Creating pool of size %lu to meet request of size %lu.\n", target_size, min_size);
    add_pool_of_size(target_size);
    return PoolView(pools[pools.size() - 1]);
}

class defer
{
public:
    defer(std::function<void()> fn) : fn(fn) {}
    ~defer() { fn(); }

private:
    std::function<void()> fn;
};

int main(int argc, char* argv[])
{
    SockLibInit();
    defer _shutdown_socklib([]() { SockLibShutdown(); });

    Socket sock(Socket::Family::INET, Socket::Type::STREAM);

    std::vector<size_t> pool_sizes;
    pool_sizes.reserve(8);
    pool_sizes.push_back(512);
    pool_sizes.push_back(512);
    pool_sizes.push_back(2048);
    pool_sizes.push_back(4096);

    init_pools(pool_sizes);

    print_allocations(true);

    if (argc > 1)
    {
        do_server(sock);
    }
    else
    {
        do_client(sock);
    }
}


void do_client(Socket& sock)
{
  Address address("127.0.0.1");

  sock.Connect(address, 8000);

  printf("Connected!\n");

  PoolView view = get_pool(4);
  OutDataStream stream(view.vector());
  stream.PutUInt32(88223399);

  //ssize_t len = sock.SendAll("Hi, server!");
  ssize_t len = sock.SendAll(stream.GetByteString());

  printf("Sent %d bytes\n", (int)len);

  PoolView recvPool = get_pool(1024);
  sock.RecvInto(recvPool.vector());

  std::cout << "Client received message '" << recvPool.vector() << "' of length " << recvPool.vector().size() << ".\n";
}

#define STR_ARGS(x) x, sizeof(x)

void do_server(Socket& sock)
{
  Address address("0.0.0.0");

  sock.Bind(address, 8000);

  printf("Bound socket successfully\n");

  sock.Listen();

  printf("Listened successfully\n");

  std::unique_ptr<Socket> connection = sock.Accept();

  printf("Accepted connection!\n");

  // ByteString msg = connection->Recv(1024);

  PoolView view = get_pool(1024);
  
  InDataStream stream(view.vector());

  connection->RecvInto(stream.buffer());

  ByteString& msg = stream.buffer();

  std::cout << "Received message of length " << msg.size() << ": '" << msg << std::endl;

  // printf("Received message of length %d: '%*s'\n", (int)msg.size(), (int)msg.size(), msg.data());

  // InDataStream stream(std::move(msg));
  int x = stream.ReadUInt32();

  std::cout << "x = " << x << std::endl;

  connection->SendAll(STR_ARGS("Hi there, client!"));
}

