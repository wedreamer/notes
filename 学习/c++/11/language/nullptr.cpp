#include <cstddef>
#include <iostream>

template <class T>
constexpr T clone(const T &t)
{
    return t;
}

void g(int *)
{
    std::cout << "函数 g 已调用\n";
}

int main()
{
    g(nullptr); // 良好
    g(NULL);    // 良好
    g(0);       // 良好

    g(clone(nullptr)); // 良好
    //  g(clone(NULL));    // 错误：非字面量的零不能为空指针常量
    //  g(clone(0));       // 错误：非字面量的零不能为空指针常量
}