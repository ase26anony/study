```cpp
// This file contains intentionally invalid C++ code designed to trigger
// specific parser error messages in GCC's parser.cc

// 1. Trigger RT_EXTERN error: missing 'extern' before linkage specification
"C" {
    void f();
}

// 2. Trigger RT_STATIC_ASSERT error: incomplete static assertion
static_assert;

// 3. Trigger RT_DECLTYPE error: decltype without parentheses
decltype x = 0;

// 4. Trigger RT_OPERATOR error: operator without specifying which operator
class MyClass {
    int operator(int);
};

// 5. Trigger RT_CLASS error: class without name but with base specifier
class : public Base {
    int x;
};

// 6. Trigger RT_TEMPLATE error: template without parameters
template int x;

// 7. Trigger RT_NAMESPACE error: namespace without valid identifier
namespace 123 {
    int y;
}

// 8. Trigger RT_USING error: using without identifier
using;

// 9. Trigger RT_ASM error: asm without string literal
asm;

// 10. Trigger RT_TRY error: try without proper compound statement
try int z = 5;
```
