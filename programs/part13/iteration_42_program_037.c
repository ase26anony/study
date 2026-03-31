```cpp
// This file contains deliberately invalid C++ constructs designed to trigger
// specific error message generation paths in GCC's parser (parser.cc).
// Each section targets a different RT_* enumeration case.

// Target: RT_EXTERN
// Linkage specification without 'extern' keyword
"C" { void f(); }

// Target: RT_STATIC_ASSERT  
// Incomplete static assertion without parentheses/condition
static_assert;

// Target: RT_DECLTYPE
// decltype without parentheses or expression
decltype x = 0;

// Target: RT_OPERATOR
// Operator overload without specifying which operator
int operator(int);

// Target: RT_CLASS
// Class definition without name but with base specifier
class : public Base { };

// Target: RT_TEMPLATE
// Template declaration without parameters
template int x;

// Target: RT_NAMESPACE
// Namespace without valid identifier
namespace 123 { }

// Target: RT_USING
// Using declaration without name
using;

// Target: RT_ASM
// Inline assembly without parentheses/string literal
asm;

// Target: RT_TRY
// Try block without catch handler
try { }
```
