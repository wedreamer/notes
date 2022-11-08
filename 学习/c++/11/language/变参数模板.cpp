#include <iostream>

void tprintf(const char *format) // 基础函数
{
    std::cout << format;
}

template <typename T, typename... Targs>
void tprintf(const char *format, T value, Targs... Fargs) // 递归变参函数
{
    for (; *format != '\0'; format++)
    {
        if (*format == '%')
        {
            std::cout << value;
            tprintf(format + 1, Fargs...); // 递归调用
            return;
        }
        std::cout << *format;
    }
}

int main()
{
    tprintf("% world% %\n", "Hello", '!', 123);
    return 0;
}