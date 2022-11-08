struct B1
{
    B1(int, ...) {}
};
struct B2
{
    B2(double) {}
};

int get();

struct D1 : B1
{
    using B1::B1; // 继承 B1(int, ...)
    int x;
    int y = get();
};

void test()
{
    D1 d(2, 3, 4); // OK：B1 通过调用 B1(2, 3, 4) 初始化，
                   // 然后 d.x 被默认初始化（不进行初始化），
                   // 然后 d.y 通过调用 get() 初始化

    // D1 e; // 错误：D1 没有默认构造函数
}

struct D2 : B2
{
    using B2::B2; // 继承 B2(double)
    B1 b;
};

// D2 f(1.0); // 错误：B1 没有默认构造函数