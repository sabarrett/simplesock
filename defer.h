#include <functional>

class defer
{
public:
    defer(std::function<void()> fn) : fn(fn) {}
    ~defer() { fn(); }

private:
    std::function<void()> fn;
};

