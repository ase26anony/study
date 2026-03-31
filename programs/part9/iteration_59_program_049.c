```cpp
// This file contains deliberate syntax errors to trigger specific
// "expected <keyword>" error messages in GCC's parser.
// Each section targets a different RT_* case from parser.cc.

// 1. Trigger RT_EXTERN error
// Linkage specification without 'extern' keyword
"C" {
    void some_function();
}

// 2. Trigger RT_STATIC_ASSERT error
// Incomplete static assertion without parentheses
static_assert;

// 3. Trigger RT_DECLTYPE error
// decltype without parentheses or expression
decltype x = 0;

// 4. Trigger RT_OPERATOR error
// Operator overload without specifying which operator
class MyClass {
    int operator(int);
};

// 5. Trigger RT_CLASS error
// Class definition without name
class : public Base {
    int member;
};

// 6. Trigger RT_TEMPLATE error
// Template declaration without angle brackets
template int y;

// 7. Trigger RT_NAMESPACE error
// Namespace without valid identifier
namespace 123 {
    int value;
}

// 8. Trigger RT_USING error
// Using declaration without name
using;

// 9. Trigger RT_ASM error
// Inline assembly without string literal
asm;

// 10. Trigger RT_TRY error
// Try block without catch handlers
try {
    int z = 42;
}
```
