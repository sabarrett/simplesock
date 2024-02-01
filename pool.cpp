#include "pool.h"
#include <math.h>
#include <stdio.h>

static std::vector<Pool> pools;

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
	    pool.pool.resize(0);
	    return PoolView(pool);
	}
    }

    // No pools large enough. Create a new one with size of the next
    // power of two.
    float fp2 = log2f((float)min_size);
    int target_size = 1 << (int)ceil(fp2);
    printf("Pools exhausted! Creating pool of size %d to meet request of size %zu.\n", target_size, min_size);
    add_pool_of_size(target_size);
    return PoolView(pools[pools.size() - 1]);
}
