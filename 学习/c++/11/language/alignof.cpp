#include <iostream>

struct Foo
{
    int i;
    float f;
    char c;
};

// 注：下面的 `alignas(alignof(long double))`
// 如果需要可以简化为  `alignas(long double)`
struct alignas(alignof(long double)) Foo2
{
    // Foo2 成员的定义...
};

struct Empty
{
};

struct alignas(64) Empty64
{
};

int main()
{
    std::cout << "对齐字节数"
                 "\n"
                 "- char                ："
              << alignof(char) << "\n"
                                  "- 指针                ："
              << alignof(int *) << "\n"
                                   "- Foo 类              ："
              << alignof(Foo) << "\n"
                                 "- Foo2 类             ："
              << alignof(Foo2) << "\n"
                                  "- 空类                ："
              << alignof(Empty) << "\n"
                                   "- 带 alignas(64) 的空类："
              << alignof(Empty64) << "\n";
}