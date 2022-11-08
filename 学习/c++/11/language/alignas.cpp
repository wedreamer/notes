// 每个 struct_float 类型对象都将被对齐到 alignof(float) 边界
// （通常为 4）：
struct alignas(alignof(float)) struct_float
{
    // 定义在此
};

// sse_t 类型的每个对象将对齐到 32 字节边界
struct alignas(32) sse_t
{
    float sse_data[4];
};

// 数组 "cacheline" 将对齐到 64 字节边界
alignas(64) char cacheline[64];

#include <iostream>
int main()
{
    struct default_aligned
    {
        float data[4];
    } a, b, c;
    sse_t x, y, z;

    std::cout
        << "alignof(struct_float) = " << alignof(struct_float) << '\n'
        << "sizeof(sse_t) = " << sizeof(sse_t) << '\n'
        << "alignof(sse_t) = " << alignof(sse_t) << '\n'
        << "alignof(cacheline) = " << alignof(alignas(64) char[64]) << '\n'
        << std::hex << std::showbase
        << "&a: " << &a << '\n'
        << "&b: " << &b << '\n'
        << "&c: " << &c << '\n'
        << "&x: " << &x << '\n'
        << "&y: " << &y << '\n'
        << "&z: " << &z << '\n';
}