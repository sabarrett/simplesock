#include <vector>
#include <stdlib.h>
#include <stdio.h>

struct Pool
{
    std::vector<char> pool;
    int lock;
};

class PoolView
{
public:
 PoolView(Pool& pool, const char* name=""):
  pool(pool),
    name(name)
    {
	pool.lock++;
    }

 PoolView(const PoolView& other):PoolView(other.pool, other.name) {}

 PoolView(PoolView&& other):PoolView(other.pool, other.name) {}

  ~PoolView()
    {
	pool.lock--;
	if (*name != '\0')
	  {
	    printf("Relinquishing pool %s\n", name);
	  }
	else
	  {
	    printf("Relinquishing pool at %p\n", this);
	  }
    }

  std::vector<char>& operator*()
    {
      return pool.pool;
    }

  std::vector<char>* operator->()
    {
      return &pool.pool;
    }

    Pool& pool;
    const char* name;
    std::vector<char>& vector() {return pool.pool;}
};

void add_pool_of_size(size_t size);
void init_pools(std::vector<size_t> sizes);
PoolView get_pool(size_t min_size);
