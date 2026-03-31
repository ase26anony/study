```cpp
// C++ source file designed to trigger specific parser error messages
// Each section contains a syntax error that should cause the parser
// to generate an "expected <keyword>" error message

// 1. Trigger RT_EXTERN error
// Linkage specification without 'extern' keyword
"C" { 
    void f(); 
}

// 2. Trigger RT_STATIC_ASSERT error  
// Incomplete static assertion without parentheses
static_assert;

// 3. Trigger RT_DECLTYPE error
// decltype without parentheses in variable declaration
decltype x = 0;

// 4. Trigger RT_OPERATOR error
// Operator overload without specifying which operator
class MyClass {
    int operator(int);
};

// 5. Trigger RT_CLASS error
// Class definition without name but with base class
class : public Base { 
    int x; 
};

// 6. Trigger RT_TEMPLATE error
// Template declaration without parameter list
template int y;

// 7. Trigger RT_NAMESPACE error
// Namespace with numeric name (invalid identifier)
namespace 123 {
    int z;
}

// 8. Trigger RT_USING error
// Using declaration without identifier
using;

// 9. Trigger RT_ASM error
// Inline assembly without string literal
asm;

// 10. Trigger RT_TRY error
// Try block without catch handler
try {
    int a = 5;
}
```
