```cpp
// This file contains deliberate syntax errors to trigger specific
// "expected <keyword>" error messages in GCC's parser.
// Compile with: g++ -fsyntax-only -std=c++17 this_file.cpp

// 1. Trigger RT_EXTERN error
// Linkage specification without 'extern'
"C" {
    void f();
}

// 2. Trigger RT_STATIC_ASSERT error
// Incomplete static assertion
static_assert;

// 3. Trigger RT_DECLTYPE error
// decltype without parentheses in invalid context
decltype x = 0;

// 4. Trigger RT_OPERATOR error
// Operator overload without operator token
class MyClass {
    int operator(int);
};

// 5. Trigger RT_CLASS error
// Class definition without name
class : public Base {
    int x;
};

// 6. Trigger RT_TEMPLATE error
// Template without parameters
template int x;

// 7. Trigger RT_NAMESPACE error
// Namespace without valid identifier
namespace 123 {
    int y;
}

// 8. Trigger RT_USING error
// Using declaration without name
using;

// 9. Trigger RT_ASM error
// Inline assembly without string literal
asm;

// 10. Trigger RT_TRY error
// Try block without catch
try {
    int z = 5;
}
```
