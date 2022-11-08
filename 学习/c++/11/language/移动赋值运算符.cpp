#include <string>
#include <iostream>
#include <utility>

struct A
{
    std::string s;

    A() : s("测试") {}

    A(const A &o) : s(o.s) { std::cout << "移动失败！\n"; }

    A(A &&o) : s(std::move(o.s)) {}

    A &operator=(const A &other)
    {
        s = other.s;
        std::cout << "复制赋值\n";
        return *this;
    }

    A &operator=(A &&other)
    {
        s = std::move(other.s);
        std::cout << "移动赋值\n";
        return *this;
    }
};

A f(A a) { return a; }

struct B : A
{
    std::string s2;
    int n;
    // 隐式移动赋值运算符 B& B::operator=(B&&)
    // 调用 A 的移动赋值运算符
    // 调用 s2 的移动赋值运算符
    // 并进行 n 的逐位复制
};

struct C : B
{
    ~C() {} // 析构函数阻止隐式移动赋值
};

struct D : B
{
    D() {}
    ~D() {}                       // 析构函数本会阻止隐式移动赋值
    D &operator=(D &&) = default; // 无论如何都强制移动赋值
};

int main()
{
    A a1, a2;
    std::cout << "尝试从右值临时量移动赋值 A\n";
    a1 = f(A()); // 从右值临时量移动赋值
    std::cout << "尝试从亡值移动赋值 A\n";
    a2 = std::move(a1); // 从亡值移动赋值

    std::cout << "尝试移动赋值 B\n";
    B b1, b2;
    std::cout << "移动前，b1.s = \"" << b1.s << "\"\n";
    b2 = std::move(b1); // 调用隐式移动赋值
    std::cout << "移动后，b1.s = \"" << b1.s << "\"\n";

    std::cout << "尝试移动赋值 C\n";
    C c1, c2;
    c2 = std::move(c1); // 调用复制赋值运算符

    std::cout << "尝试移动赋值 D\n";
    D d1, d2;
    d2 = std::move(d1);
}