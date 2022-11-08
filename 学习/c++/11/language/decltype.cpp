// https://zh.cppreference.com/w/cpp/language/decltype
#include <iostream>
#include <type_traits>

struct A
{
    double x;
};
const A *a;

decltype(a->x) y;       // y 的类型是 double（其声明类型）
decltype((a->x)) z = y; // z 的类型是 const double&（左值表达式）

template <typename T, typename U>
auto add(T t, U u) -> decltype(t + u) // 返回类型依赖于模板形参
{                                     // C++14 开始可以推导返回类型
    return t + u;
}

int main()
{
    int i = 33;
    decltype(i) j = i * 2;

    std::cout << "i = " << i << ", "
              << "j = " << j << '\n';

    std::cout << "i 和 j 的类型相同吗？"
              << (std::is_same_v<decltype(i), decltype(j)> ? "相同" : "不同") << '\n';

    auto f = [](int a, int b) -> int
    {
        return a * b;
    };

    decltype(f) g = f; // lambda 的类型是独有且无名的
    i = f(2, 2);
    j = g(3, 3);

    std::cout << "i = " << i << ", "
              << "j = " << j << '\n';
}