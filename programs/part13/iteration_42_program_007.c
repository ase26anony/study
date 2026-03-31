```cpp
// This file contains deliberately erroneous C++ constructs designed to trigger
// specific error-handling paths in GCC's parser (parser.cc).
// Each section targets a different RT_* enumeration case.

// 1. Trigger RT_EXTERN error: linkage specification without 'extern'
"C" { void f(); }

// 2. Trigger RT_STATIC_ASSERT error: incomplete static assertion
static_assert;

// 3. Trigger RT_DECLTYPE error: decltype without parentheses
decltype x = 0;

// 4. Trigger RT_OPERATOR error: operator without specific operator
class MyClass {
    int operator(int);
};

// 5. Trigger RT_CLASS error: class definition without name
class : public Base { };

// 6. Trigger RT_TEMPLATE error: template without parameters
template int x;

// 7. Trigger RT_NAMESPACE error: namespace without valid identifier
namespace 123 { }

// 8. Trigger RT_USING error: using without namespace/identifier
using;

// 9. Trigger RT_ASM error: asm without string literal
asm;

// 10. Trigger RT_TRY error: try without proper catch block
try { }
```
