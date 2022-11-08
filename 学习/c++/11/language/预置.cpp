// trivial 的所有特殊成员函数都分别在它们的首个声明处被显式预置，
// 因此它们都不由用户提供
struct trivial
{
    trivial() = default;
    trivial(const trivial &) = default;
    trivial(trivial &&) = default;
    trivial &operator=(const trivial &) = default;
    trivial &operator=(trivial &&) = default;
    ~trivial() = default;
};

struct nontrivial
{
    nontrivial(); // 首个声明
};

// 没有在首个声明处被显式预置，
// 因此该函数由用户提供并在此定义
nontrivial::nontrivial() = default;