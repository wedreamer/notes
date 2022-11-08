#include <aio.h>
#include <cstddef>

struct sometype
{
    void *operator new(std::size_t) = delete;
    void *operator new[](std::size_t) = delete;
};

// sometype *p = new sometype; // 错误：尝试调用弃置的 sometype::operator new

struct sometype
{
    sometype();
};
sometype::sometype() = delete; // 错误：必须在首条声明弃置
