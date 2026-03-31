```cpp
// This file contains intentionally invalid C++ code designed to trigger
// specific error message generation paths in the GCC parser.
// Each section corresponds to one of the RT_* cases in parser.cc lines 34457-34486.

// RT_EXTERN: Linkage specification without 'extern' keyword
"C" { void f(); }  // Error: expected 'extern' before string literal

// RT_STATIC_ASSERT: Incomplete static assertion
static_assert;  // Error: expected '(' before ';' token

// RT_DECLTYPE: decltype without proper parentheses
decltype x = 0;  // Error: expected '(' before 'x'

// RT_OPERATOR: operator overload without operator token
int operator(int);  // Error: expected type-specifier before 'operator'

// RT_CLASS: Class definition without name
class : public Base { };  // Error: expected identifier before ':' token

// RT_TEMPLATE: Template declaration without parameters
template int x;  // Error: expected '<' before 'int'

// RT_NAMESPACE: Namespace without valid identifier
namespace = alias;  // Error: expected identifier before '=' token

// RT_USING: Using declaration without name
using;  // Error: expected namespace-name or qualified-id

// RT_ASM: Inline assembly without string literal
asm;  // Error: expected string-literal before ';' token

// RT_TRY: Try block without catch handler
try { int x = 5; }  // Error: expected 'catch' before end of file
```
