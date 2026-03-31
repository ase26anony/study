```cpp
// C++ source file designed to trigger specific parser error messages
// Each section contains a syntax error that should cause the parser
// to generate "expected <keyword>" error messages

// 1. Trigger RT_EXTERN error: missing 'extern' in linkage specification
"C" {
    void foreign_func();
}

// 2. Trigger RT_STATIC_ASSERT error: incomplete static assertion
static_assert;
static_assert "message";

// 3. Trigger RT_DECLTYPE error: decltype without parentheses
decltype x = 0;
template<typename T>
typename decltype;

// 4. Trigger RT_OPERATOR error: operator without specifying which operator
class MyClass {
    int operator(int);
    MyClass::operator { return 0; }
};

// 5. Trigger RT_CLASS error: class definition without name
class : public Base {
    int x;
};

struct {
    int y;
} anonymous;

// 6. Trigger RT_TEMPLATE error: template without parameters
template int global_var;
template {
    void func();
}

// 7. Trigger RT_NAMESPACE error: namespace without valid name
namespace = alias;
namespace 123 {
    int value;
}

// 8. Trigger RT_USING error: using without identifier
using;
using namespace = std;

// 9. Trigger RT_ASM error: asm without assembly string
asm;
asm 123;

// 10. Trigger RT_TRY error: try without proper catch
try int x = 5;
try {
    throw 42;
}

// Additional context to ensure parser attempts all constructs
typedef int myint;

// End of file
```
