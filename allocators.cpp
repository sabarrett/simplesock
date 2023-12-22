#include <cstdlib>
#include <iostream>
#include <memory>
#include <new>

static bool should_print = false;

void print_allocations(bool to_print)
{
    should_print = to_print;
}

void* operator new(std::size_t n) noexcept(false) //throw(std::bad_alloc)
{
    void* p = std::malloc(n);
    if (p == nullptr)
    {
	throw std::bad_alloc();
    }

    if (should_print)
    {
        std::cout << "Allocated " << n << " bytes at " << std::hex << p << std::dec << "\n";
    }

    return p;
}
