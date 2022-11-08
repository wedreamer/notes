#include <iostream>

enum class Color
{
    red,
    green = 20,
    blue
};
Color r = Color::blue;

int main(int argc, char const *argv[])
{
    switch (r)
    {
    case Color::red:
        std::cout << "红\n";
        break;
    case Color::green:
        std::cout << "绿\n";
        break;
    case Color::blue:
        std::cout << "蓝\n";
        break;
    }
    return 0;
}

// int n = r; // 错误：不存在从有作用域枚举到 int 的隐式转换
int n = static_cast<int>(r); // OK, n = 21

enum class fruit
{
    orange,
    apple
};

struct S
{
    using enum fruit; // OK ：引入 orange 与 apple 到 S 中
};

void f()
{
    S s;
    s.orange;  // OK ：指名 fruit::orange
    S::orange; // OK ：指名 fruit::orange
}