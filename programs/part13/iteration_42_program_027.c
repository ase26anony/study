```cpp
// This file contains deliberately erroneous C++ constructs designed to trigger
// specific error message generation paths in GCC's parser (parser.cc).
// Each section targets a different RT_* enumeration case for error reporting.
// Compile with: g++ -fsyntax-only -std=c++17 this_file.cpp

// 1. Trigger RT_EXTERN error: linkage specification without 'extern'
"C" {
    void some_function();
}

// 2. Trigger RT_STATIC_ASSERT error: incomplete static assertion
static_assert;

// 3. Trigger RT_DECLTYPE error: decltype without parentheses or expression
decltype x = 0;

// 4. Trigger RT_OPERATOR error: operator overload without operator token
class MyClass {
    int operator(int);
};

// 5. Trigger RT_CLASS error: class definition without name
class : public Base {
    int member;
};

// 6. Trigger RT_TEMPLATE error: template without parameters
template int y;

// 7. Trigger RT_NAMESPACE error: namespace without valid identifier
namespace 123 {
    int value;
}

// 8. Trigger RT_USING error: using declaration without name
using;

// 9. Trigger RT_ASM error: asm statement without string literal
asm;

// 10. Trigger RT_TRY error: try block without catch
try {
    int z = 42;
}
```
