class Foo
{
public:
    Foo(char x, int y) {}
    Foo(int y) : Foo('a', y) {} // Foo(int) 委托到 Foo(char, int)
};