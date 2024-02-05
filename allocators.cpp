#include <cstdlib>
#include <new>
#include <limits>
#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>

#include "allocators.h"

static bool allocs_should_print = true;

static unsigned int base_alloc_counter = 0;
static unsigned int *p_alloc_counter = &base_alloc_counter;

void set_allocs_should_print(bool should_print)
{
  allocs_should_print = should_print;
}

template<class T>
struct Mallocator
{
    // Class taken from https://en.cppreference.com/w/cpp/named_req/Allocator
    typedef T value_type;

    Mallocator () = default;

    template<class U>
    constexpr Mallocator (const Mallocator <U>&) noexcept {}

    [[nodiscard]] T* allocate(std::size_t n)
    {
        if (n > std::numeric_limits<std::size_t>::max() / sizeof(T))
            throw std::bad_array_new_length();


        if (auto p = static_cast<T*>(std::malloc(n * sizeof(T))))
        {
            //report(p, n);
            return p;
        }

        throw std::bad_alloc();
    }

    void deallocate(T* p, std::size_t n) noexcept
    {
        //report(p, n, 0);
        std::free(p);
    }
private:
    void report(T* p, std::size_t n, bool alloc = true) const
    {
        std::cout << (alloc ? "Alloc: " : "Dealloc: ") << sizeof(T) * n
                  << " bytes at " << std::hex << std::showbase
                  << reinterpret_cast<void*>(p) << std::dec << '\n';
    }
};

template<class T, class U>
bool operator==(const Mallocator <T>&, const Mallocator <U>&) { return true; }

template<class T, class U>
bool operator!=(const Mallocator <T>&, const Mallocator <U>&) { return false; }

static std::unordered_map<const void*, std::size_t, std::hash<const void *>, std::equal_to<const void*>, Mallocator<std::pair<const void* const, std::size_t>>> allocs;

void * operator new(std::size_t n)
{
  void *p = std::malloc(n);
  if (p == nullptr)
      throw std::bad_alloc();
  allocs[p] = n;
  (*p_alloc_counter)++;
  if (allocs_should_print)
  {
      std::cout << "  Alloc: " << n << " bytes at " << std::hex << std::showbase << p << std::dec << '\n';
  }
  return p;
}
void operator delete(void * p) throw()
{
  std::size_t n = allocs.at(p);
  if (allocs_should_print)
  {
      std::cout << "Dealloc: " << n << " bytes at " << std::hex << std::showbase << p << std::dec << '\n';
  }
  std::free(p);
  allocs.erase(p);
}

// Instance of a class that reports leaks when destroyed.
// Statically constructed, so will be destroyed when
// program exits (unless it crashes)
static class Reporter
{
public:
  ~Reporter()
{
  bool hasLeak = false;
  std::cout << "==== Begin Leak Report ====\n";
  for (auto & p : allocs)
    {
      hasLeak = true;
      std::cout << "Leak: " << p.second << " bytes at " << std::hex << std::showbase << p.first << '\n';
    }
  if (!hasLeak)
    std::cout << "✨ No leaks detected ✨\n";
}
} __reporter__;


AllocationContext::AllocationContext(const char* name)
{
    _alloc_counter = 0;
    _p_last_alloc_counter = p_alloc_counter;
    p_alloc_counter = &_alloc_counter;
    strlcpy(_name, name, sizeof(_name));
    std::cout << "vvvvv Beginning Allocation Context '" << _name << "' vvvvv\n";
}

AllocationContext::~AllocationContext()
{
    p_alloc_counter = _p_last_alloc_counter;
    (*_p_last_alloc_counter) += _alloc_counter;
    std::cout << "^^^^^ END '" << _name << "' (" << _alloc_counter << " allocations) ^^^^^\n";
}
