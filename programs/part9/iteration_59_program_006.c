```cpp
// Coverage test for GCC parser error messages
// This file contains deliberate syntax errors to trigger specific "expected" error messages

// 1. Trigger RT_EXTERN error
// Linkage specification without 'extern' keyword
"C" {
    void f1();
}

// 2. Trigger RT_STATIC_ASSERT error
// Incomplete static assertion without parentheses
static_assert;

// 3. Trigger RT_DECLTYPE error
// decltype without parentheses in variable declaration
decltype x = 0;

// 4. Trigger RT_OPERATOR error
// Operator overload without specifying which operator
struct MyClass {
    int operator(int);
};

// 5. Trigger RT_CLASS error
// Class definition without name but with base specifier
class : public Base {
    int x;
};

// 6. Trigger RT_TEMPLATE error
// Template declaration without angle brackets
template int x;

// 7. Trigger RT_NAMESPACE error
// Namespace without valid identifier
namespace 123 {
    int y;
}

// 8. Trigger RT_USING error
// Using declaration without identifier
using;

// 9. Trigger RT_ASM error
// Inline assembly without parentheses or string literal
asm;

// 10. Trigger RT_TRY error
// Try block without catch handlers
try {
    int z = 5;
}

// Additional variations to ensure coverage

// Another static_assert error
static_assert "message";

// Another decltype error in template context
template<typename T = decltype>
class AnotherClass;

// Another operator error
MyClass::operator { return 0; }

// Another namespace error
namespace = alias;

// Another using error
using namespace = std;

// Another asm error
asm 123;

// Another try error
try int w = 10;
```
