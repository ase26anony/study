```cpp
// C++ source file designed to trigger specific parser error messages
// Each section contains a syntax error that should cause the parser
// to generate an "expected <keyword>" error for the uncovered lines.

// 1. Trigger RT_EXTERN error
// Linkage specification without 'extern' keyword
"C" { void f(); }

// 2. Trigger RT_STATIC_ASSERT error  
// Incomplete static assertion without parentheses
static_assert;

// 3. Trigger RT_DECLTYPE error
// decltype without parentheses in invalid context
decltype x = 0;

// 4. Trigger RT_OPERATOR error
// Operator overload without specifying which operator
int operator(int);

// 5. Trigger RT_CLASS error
// Class definition without name but with base specifier
class : public Base { };

// 6. Trigger RT_TEMPLATE error
// Template declaration without parameter list
template int x;

// 7. Trigger RT_NAMESPACE error
// Namespace without valid identifier
namespace = alias;

// 8. Trigger RT_USING error
// Using declaration without name
using;

// 9. Trigger RT_ASM error
// Inline assembly without string literal
asm;

// 10. Trigger RT_TRY error
// Try block without catch handler
try { int x = 5; }
```
