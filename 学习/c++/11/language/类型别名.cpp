#include <string>
#include <ios>
#include <type_traits>

// 类型别名，等同于
// typedef std::ios_base::fmtflags flags;
using flags = std::ios_base::fmtflags;
// 名字 'flags' 现在指代类型：
flags fl = std::ios_base::dec;

// 类型别名，等同于
// typedef void (*func)(int, int);
using func = void (*)(int, int);
// 名字 'func' 现在指代函数指针：
void example(int, int) {}
func f = example;

// 别名模板
template <class T>
using ptr = T *;
// 名字 'ptr<T>' 现在是指向 T 的指针的别名
ptr<int> x;

// 用于隐藏模板形参的别名模版
template <class CharT>
using mystring = std::basic_string<CharT, std::char_traits<CharT>>;
mystring<char> str;

// 别名模板可引入成员 typedef 名
template <typename T>
struct Container
{
    using value_type = T;
};
// 可用于泛型编程
template <typename ContainerType>
void g(const ContainerType &c) { typename ContainerType::value_type n; }

// 用于简化 std::enable_if 语法的类型别名
template <typename T>
using Invoke = typename T::type;
template <typename Condition>
using EnableIf = Invoke<std::enable_if<Condition::value>>;
template <typename T, typename = EnableIf<std::is_polymorphic<T>>>
int fpoly_only(T t) { return 1; }

struct S
{
    virtual ~S() {}
};

int main()
{
    Container<int> c;
    g(c); // Container::value_type 将在此函数中是 int
          //  fpoly_only(c); // 错误：被 enable_if 禁止
    S s;
    fpoly_only(s); // OK：被 enable_if 允许
}