#include <iostream>
#include <string>

int main()
{
    std::string s1 = "Test";
    //  std::string&& r1 = s1;           // 错误：不能绑定到左值

    const std::string &r2 = s1 + s1; // OK：到 const 的左值引用延长生存期
                                     //  r2 += "Test";                    // 错误：不能通过到 const 的引用修改

    std::string &&r3 = s1 + s1; // OK：右值引用延长生存期
    r3 += "Test";               // OK：能通过到非 const 的引用修改
    std::cout << r3 << '\n';
}