```cpp
// C++ source file designed to trigger specific parser error messages
// Each section contains a syntax error that should cause the parser
// to generate an "expected <keyword>" error for the uncovered lines.

// 1. Trigger RT_EXTERN error: missing 'extern' before linkage specifier
"C" { 
    void foreign_func(); 
}

// 2. Trigger RT_STATIC_ASSERT error: incomplete static assertion
static_assert;

// 3. Trigger RT_DECLTYPE error: decltype without parentheses
decltype x = 0;

// 4. Trigger RT_OPERATOR error: operator without specifying which operator
struct MyClass {
    int operator(int);
};

// 5. Trigger RT_CLASS error: class definition without name
class : public Base {
    int member;
};

// 6. Trigger RT_TEMPLATE error: template without parameters
template int global_var;

// 7. Trigger RT_NAMESPACE error: namespace without valid identifier
namespace 123 {
    int value;
}

// 8. Trigger RT_USING error: using declaration without identifier
using;

// 9. Trigger RT_ASM error: asm statement without assembly string
asm;

// 10. Trigger RT_TRY error: try without catch block
try {
    int x = 5;
}
```
