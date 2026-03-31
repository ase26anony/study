```cpp
// This file contains deliberately invalid C++ code designed to trigger
// specific "expected keyword" error messages in GCC's parser.
// Each section targets a different RT_* case from parser.cc lines 34457-34486.

// 1. Trigger RT_EXTERN error: missing 'extern' in linkage specification
"C" {
    void f();
}

// 2. Trigger RT_STATIC_ASSERT error: incomplete static assertion
static_assert;

// 3. Trigger RT_DECLTYPE error: invalid decltype usage
decltype x = 0;

// 4. Trigger RT_OPERATOR error: incomplete operator overload
class MyClass {
    int operator(int);
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

// 8. Trigger RT_USING error: incomplete using declaration
using;

// 9. Trigger RT_ASM error: inline assembly without proper syntax
asm;

// 10. Trigger RT_TRY error: try block without catch
try {
    int z = 0;
}
```
