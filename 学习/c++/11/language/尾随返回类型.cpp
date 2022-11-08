// https://zh.cppreference.com/w/cpp/language/auto
#include <iostream>
#include <utility>

/* 
(C++ 11 起) 尾随返回类型，当返回类型取决于实参名时，例如 
    template <class T, class U>
    auto add(T t, U u) -> decltype(t + u);
，或当返回类型复杂时，例如在 
    auto fpif(int) -> int (*)(int) 中，尾随返回类型很有用 
*/

    template <class T, class U>
    auto add(T t, U u)
{
    return t + u;
} // 返回类型是 operator+(T, U) 的类型

// 在它调用的函数返回引用的情况下
// 函数调用的完美转发必须用 decltype(auto)
template <class F, class... Args>
decltype(auto) PerfectForward(F fun, Args &&...args)
{
    return fun(std::forward<Args>(args)...);
}

template <auto n>                               // C++17 auto 形参声明
auto f() -> std::pair<decltype(n), decltype(n)> // auto 不能从花括号初始化器列表推导
{
    return {n, n};
}