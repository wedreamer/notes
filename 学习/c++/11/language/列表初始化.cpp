#include <iostream>
#include <vector>
#include <map>
#include <string>

struct Foo
{
    std::vector<int> mem = {1, 2, 3}; // 非静态成员的列表初始化
    std::vector<int> mem2;
    Foo() : mem2{-1, -2, -3} {} // 构造函数中的成员列表初始化
};

std::pair<std::string, std::string> f(std::pair<std::string, std::string> p)
{
    return {p.second, p.first}; // return 语句中的列表初始化
}

int main()
{
    int n0{};  // 值初始化（为零）
    int n1{1}; // 直接列表初始化

    std::string s1{'a', 'b', 'c', 'd'}; // initializer_list 构造函数调用
    std::string s2{s1, 2, 2};           // 常规构造函数调用
    std::string s3{0x61, 'a'};          // initializer_list 构造函数偏好 (int, char)

    int n2 = {1};                        // 复制列表初始化
    double d = double{1.2};              // 纯右值的列表初始化，然后复制初始化
    auto s4 = std::string{"HelloWorld"}; // 同上， C++17 起不创建临时对象

    std::map<int, std::string> m = // 嵌套列表初始化
        {
            {1, "a"},
            {2, {'a', 'b', 'c'}},
            {3, s1}};

    std::cout << f({"hello", "world"}).first // 函数调用中的列表初始化
              << '\n';

    const int(&ar)[2] = {1, 2}; // 绑定左值引用到临时数组
    int &&r1 = {1};             // 绑定右值引用到临时 int
                                //  int& r2 = {2}; // 错误：不能绑定右值到非 const 左值引用

    //  int bad{1.0}; // 错误：窄化转换
    unsigned char uc1{10}; // 可以
                           //  unsigned char uc2{-1}; // 错误：窄化转换

    Foo f;

    std::cout << n0 << ' ' << n1 << ' ' << n2 << '\n'
              << s1 << ' ' << s2 << ' ' << s3 << '\n';
    for (auto p : m)
        std::cout << p.first << ' ' << p.second << '\n';
    for (auto n : f.mem)
        std::cout << n << ' ';
    for (auto n : f.mem2)
        std::cout << n << ' ';
}