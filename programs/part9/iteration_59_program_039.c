```cpp
// This file contains deliberate syntax errors to trigger specific
// "expected <keyword>" error messages in GCC's parser.
// Each section targets a different RT_* case from parser.cc lines 34457-34486.

// 1. Trigger RT_EXTERN error: expected "extern"
// Linkage specification without 'extern' keyword
"C" {
    void some_function();
}

// 2. Trigger RT_STATIC_ASSERT error: expected "static_assert"
// Incomplete static assertion (missing parentheses and condition)
static_assert;

// 3. Trigger RT_DECLTYPE error: expected "decltype"
// decltype without parentheses in variable declaration
decltype x = 0;

// 4. Trigger RT_OPERATOR error: expected "operator"
// Operator overload without specifying which operator
class MyClass {
    int operator(int);
};

// 5. Trigger RT_CLASS error: expected "class"
// Class definition without name but with inheritance
class : public Base {
    int x;
};

// 6. Trigger RT_TEMPLATE error: expected "template"
// Template declaration without angle brackets
template int y;

// 7. Trigger RT_NAMESPACE error: expected "namespace"
// Namespace definition with invalid name
namespace 123 {
    int z;
}

// 8. Trigger RT_USING error: expected "using"
// Using declaration without identifier
using;

// 9. Trigger RT_ASM error: expected "asm"
// Inline assembly without parentheses or string literal
asm;

// 10. Trigger RT_TRY error: expected "try"
// Try block without catch handlers
try {
    int a = 5;
}
```
