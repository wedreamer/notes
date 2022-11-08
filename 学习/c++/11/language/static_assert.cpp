#include <type_traits>

template <class T>
void swap(T &a, T &b) noexcept
{
    static_assert(std::is_copy_constructible<T>::value,
                  "交换需要可复制");
    static_assert(std::is_nothrow_copy_constructible<T>::value && std::is_nothrow_copy_assignable<T>::value,
                  "交换需要可复制及可赋值，且无异常抛出");
    auto c = b;
    b = a;
    a = c;
}

template <class T>
struct data_structure
{
    static_assert(std::is_default_constructible<T>::value,
                  "数据结构要求元素可默认构造");
};

struct no_copy
{
    no_copy(const no_copy &) = delete;
    no_copy() = default;
};

struct no_default
{
    no_default() = delete;
};

int main()
{
    int a, b;
    swap(a, b);

    no_copy nc_a, nc_b;
    swap(nc_a, nc_b); // 1

    data_structure<int> ds_ok;
    data_structure<no_default> ds_error; // 2
}