#include <cstdlib>
#include <iostream>
#include <memory>
#include <new>

void* operator new(std::size_t n) noexcept(false) //throw(std::bad_alloc)
{
    void* p = std::malloc(n);
    if (p == nullptr)
    {
	throw std::bad_alloc();
    }

    std::cout << "Allocated " << n << " bytes at " << std::hex << p << std::dec << "\n";

    return p;
}
