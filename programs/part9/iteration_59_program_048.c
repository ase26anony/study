```cpp
// This file contains deliberate syntax errors to trigger specific
// "expected" error messages in GCC's parser.cc

// 1. Trigger RT_EXTERN error: linkage specification without 'extern'
"C" {
    void f();
}

// 2. Trigger RT_STATIC_ASSERT error: incomplete static assertion
static_assert;

// 3. Trigger RT_DECLTYPE error: decltype without parentheses
decltype x = 0;

// 4. Trigger RT_OPERATOR error: operator without specifying which operator
struct MyClass {
    int operator;
};

// 5. Trigger RT_CLASS error: class definition without name
class : public Base {
    int x;
};

// 6. Trigger RT_TEMPLATE error: template without parameters
template int x;

// 7. Trigger RT_NAMESPACE error: namespace without valid name
namespace 123 {
    int y;
}

// 8. Trigger RT_USING error: using without identifier
using;

// 9. Trigger RT_ASM error: asm without assembly string
asm;

// 10. Trigger RT_TRY error: try without catch
try {
    int z = 5;
}
```
