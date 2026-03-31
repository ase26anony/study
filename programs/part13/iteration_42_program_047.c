```cpp
// This file contains intentionally invalid C++ code designed to trigger
// specific error message generation paths in GCC's parser.
// Each section targets a different RT_* case from parser.cc.

// Target: RT_EXTERN
// Linkage specification without 'extern' keyword
"C" { void f(); }

// Target: RT_STATIC_ASSERT  
// Incomplete static assertion without parentheses
static_assert;

// Target: RT_DECLTYPE
// decltype without parentheses in invalid context
decltype x = 0;

// Target: RT_OPERATOR
// Operator overload without specifying which operator
int operator(int);

// Target: RT_CLASS
// Class definition without name but with base specifier
class : public Base { };

// Target: RT_TEMPLATE
// Template declaration without parameter list
template int x;

// Target: RT_NAMESPACE
// Namespace without valid identifier
namespace = alias;

// Target: RT_USING
// Using declaration without name
using;

// Target: RT_ASM
// Inline assembly without parentheses or string literal
asm;

// Target: RT_TRY
// Try block without catch handler
try { }
```
