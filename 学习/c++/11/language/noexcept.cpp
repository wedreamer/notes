/* // foo 是否声明为 noexcept 取决于 T() 是否会抛出异常
template <class T>
void foo() noexcept(noexcept(T())) {}

void bar() noexcept(true) {}
void baz() noexcept { throw 42; } // noexcept 等同于 noexcept(true)

int main()
{
    foo<int>(); // noexcept(noexcept(int())) => noexcept(true)，所以这是可以的

    bar(); // 可以
    baz(); // 能编译，但会在运行时调用 std::terminate
} */

#include <iostream>
#include <utility>
#include <vector>

void may_throw();
void no_throw() noexcept;
auto lmay_throw = [] {};
auto lno_throw = []() noexcept {};

class T
{
public:
    ~T() {} // 析构函数妨碍了移动构造函数
            // 复制构造函数不会抛出异常
};

class U
{
public:
    ~U() {} // 析构函数妨碍了移动构造函数
            // 复制构造函数可能会抛出异常
    std::vector<int> v;
};

class V
{
public:
    std::vector<int> v;
};

int main()
{
    T t;
    U u;
    V v;

    std::cout << std::boolalpha
              << "may_throw() 可能会抛出异常吗？" << !noexcept(may_throw()) << '\n'
              << "no_throw() 可能会抛出异常吗？" << !noexcept(no_throw()) << '\n'
              << "lmay_throw() 可能会抛出异常吗？" << !noexcept(lmay_throw()) << '\n'
              << "lno_throw() 可能会抛出异常吗？" << !noexcept(lno_throw()) << '\n'
              << "~T() 可能会抛出异常吗？" << !noexcept(std::declval<T>().~T()) << '\n'
              // 注：以下各项测试也要求 ~T() 不会抛出异常
              // 因为 noexcept 中的表达式会构造并销毁临时量
              << "T(T 右值) 可能会抛出异常吗？" << !noexcept(T(std::declval<T>())) << '\n'
              << "T(T 左值) 可能会抛出异常吗？" << !noexcept(T(t)) << '\n'
              << "U(U 右值) 可能会抛出异常吗？" << !noexcept(U(std::declval<U>())) << '\n'
              << "U(U 左值) 可能会抛出异常吗？" << !noexcept(U(u)) << '\n'
              << "V(V 右值) 可能会抛出异常吗？" << !noexcept(V(std::declval<V>())) << '\n'
              << "V(V 左值) 可能会抛出异常吗？" << !noexcept(V(v)) << '\n';
}