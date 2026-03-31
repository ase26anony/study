```cpp
// This file contains deliberately erroneous C++ constructs designed to trigger
// specific error message generation paths in GCC's parser (parser.cc).
// Each section targets a different RT_* case from lines 34457-34486.

// Target RT_EXTERN: Linkage specification without 'extern'
"C" { void f(); }

// Target RT_STATIC_ASSERT: Incomplete static assertion
static_assert;

// Target RT_DECLTYPE: decltype without parentheses in invalid context
decltype x = 0;

// Target RT_OPERATOR: operator overload without specifying which operator
int operator(int);

// Target RT_CLASS: Class definition without name but with base specifier
class : public Base { };

// Target RT_TEMPLATE: template without parameters
template int x;

// Target RT_NAMESPACE: namespace without valid identifier
namespace 123 { }

// Target RT_USING: using declaration without name
using;

// Target RT_ASM: asm statement without string literal
asm;

// Target RT_TRY: try without catch
try { }
```
