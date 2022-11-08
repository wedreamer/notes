struct A;
struct A final
{
}; // OK：定义结构体 A，而不是值初始化变量 final

struct X
{
    struct C
    {
        constexpr operator int() { return 5; }
    };
    struct B final : C
    {
    }; // OK：定义嵌套类 B，而不是声明位域成员 final
};

struct Base
{
    virtual void foo();
};

struct A : Base
{
    void foo() final; // Base::foo 被覆盖而 A::foo 是最终覆盖函数
    // void bar() final; // 错误：bar 非虚，因此它不能是 final 的
};

struct B final : A // struct B 为 final
{
    // void foo() override; // 错误：foo 不能被覆盖，因为它在 A 中是 final 的
};

// struct C : B
// {
// }; // 错误：B 是 final 的