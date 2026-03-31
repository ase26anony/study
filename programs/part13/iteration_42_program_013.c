```cpp
// This file contains intentionally invalid C++ code designed to trigger
// specific error message generation paths in GCC's parser.
// Each section targets a different RT_* enumeration case from parser.cc.

// 1. Trigger RT_EXTERN error: linkage specification without 'extern'
"C" { void f(); }

// 2. Trigger RT_STATIC_ASSERT error: incomplete static assertion
static_assert;

// 3. Trigger RT_DECLTYPE error: decltype without proper parentheses
decltype x = 0;

// 4. Trigger RT_OPERATOR error: operator overload without operator token
int operator(int);

// 5. Trigger RT_CLASS error: class definition without name
class : public Base { };

// 6. Trigger RT_TEMPLATE error: template without parameter list
template int x;

// 7. Trigger RT_NAMESPACE error: namespace without valid identifier
namespace = alias;

// 8. Trigger RT_USING error: using declaration without name
using;

// 9. Trigger RT_ASM error: asm statement without string literal
asm;

// 10. Trigger RT_TRY error: try block without proper structure
try int x = 5;
```
