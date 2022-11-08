#include <string>
#include <iostream>
#include <iomanip>
#include <utility>

struct A
{
    std::string s;
    int k;

    A() : s("测试"), k(-1) {}
    A(const A &o) : s(o.s), k(o.k) { std::cout << "移动失败！\n"; }
    A(A &&o)
    noexcept : s(std::move(o.s)),       // 类类型成员的显式移动
               k(std::exchange(o.k, 0)) // 非类类型成员的显式移动
    {
    }
};

A f(A a)
{
    return a;
}

struct B : A
{
    std::string s2;
    int n;
    // 隐式移动构造函数 B::(B&&)
    // 调用 A 的移动构造函数
    // 调用 s2 的移动构造函数
    // 并进行 n 的逐位复制
};

struct C : B
{
    ~C() {} // 析构函数阻止隐式移动构造函数 C::(C&&)
};

struct D : B
{
    D() {}
    ~D() {}            // 析构函数阻止隐式移动构造函数 D::(D&&)
    D(D &&) = default; // 强制生成移动构造函数
};

int main()
{
    std::cout << "尝试移动 A\n";
    A a1 = f(A()); // 按值返回时，从函数形参移动构造其目标
    std::cout << "移动前，a1.s = " << std::quoted(a1.s)
              << " a1.k = " << a1.k << '\n';
    A a2 = std::move(a1); // 从亡值移动构造
    std::cout << "移动后，a1.s = " << std::quoted(a1.s)
              << " a1.k = " << a1.k << '\n';

    std::cout << "尝试移动 B\n";
    B b1;
    std::cout << "移动前，b1.s = " << std::quoted(b1.s) << "\n";
    B b2 = std::move(b1); // 调用隐式移动构造函数
    std::cout << "移动后，b1.s = " << std::quoted(b1.s) << "\n";

    std::cout << "尝试移动 C\n";
    C c1;
    C c2 = std::move(c1); // 调用复制构造函数

    std::cout << "尝试移动 D\n";
    D d1;
    D d2 = std::move(d1);
}