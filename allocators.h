#include <string>

void set_allocs_should_print(bool should_print);

class AllocationContext
{
    public:
    AllocationContext(const char* name=nullptr);
    ~AllocationContext();

    char _name[32];
    unsigned int _alloc_counter;
    unsigned int* _p_last_alloc_counter;
};
