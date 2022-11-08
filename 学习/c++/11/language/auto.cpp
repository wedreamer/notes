// https://zh.cppreference.com/w/cpp/language/auto
#include <iostream>
#include <utility>

template <class T, class U>
auto add(T t, U u) { return t + u; } // 返回类型是 operator+(T, U) 的类型

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

int main()
{
    auto a = 1 + 2;       // a 的类型是 int
    auto b = add(1, 1.2); // b 的类型是 double
    static_assert(std::is_same_v<decltype(a), int>);
    static_assert(std::is_same_v<decltype(b), double>);

    auto c0 = a;             // c0 的类型是 int，保有 a 的副本
    decltype(auto) c1 = a;   // c1 的类型是 int，保有 a 的副本
    decltype(auto) c2 = (a); // c2 的类型是 int&，它是 a 的别名
    std::cout << "通过 c2 修改前，a = " << a << '\n';
    ++c2;
    std::cout << "通过 c2 修改后，a = " << a << '\n';

    auto [v, w] = f<0>(); // 结构化绑定声明

    auto d = {1, 2}; // OK：d 的类型是 std::initializer_list<int>
    auto n = {5};    // OK：n 的类型是 std::initializer_list<int>
                     //  auto e{1, 2};    // C++17 起错误，之前是 std::initializer_list<int>
    auto m{5};       // OK：DR N3922 起 m 的类型是 int，之前是 initializer_list<int>
                     //  decltype(auto) z = { 1, 2 } // 错误：{1, 2} 不是表达式

    // auto 常用于无名类型，例如 lambda 表达式的类型
    auto lambda = [](int x)
    { return x + 3; };

    //  auto int x; // 在 C++98 合法，C++11 起错误
    //  auto x;     // 在 C 合法，在 C++ 错误

    [](...) {}(c0, c1, v, w, d, n, m, lambda); // 阻止“变量未使用”警告
}
