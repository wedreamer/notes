/* "s字符序列(可选)"	(1)	
L"s字符序列(可选)"	(2)	
u8"s字符序列(可选)"	(3)	(C++11 起)
u"s字符序列(可选)"	(4)	(C++11 起)
U"s字符序列(可选)"	(5)	(C++11 起)
前缀(可选) R"d字符序列(可选)(r字符序列(可选))d字符序列(可选)"	(6)	(C++11 起)
 */

#include <iostream>

char array1[] = "Foo"
                "bar";
// 同
char array2[] = {'F', 'o', 'o', 'b', 'a', 'r', '\0'};

const char *s1 = R"foo(
Hello
  World
)foo";
// 同
const char *s2 = "\nHello\n  World\n";
// 同
const char *s3 = "\n"
                 "Hello\n"
                 "  World\n";
int main()
{
    std::cout << array1 << '\n';
    std::cout << array2 << '\n';
    std::cout << s1 << s2 << s3;
}