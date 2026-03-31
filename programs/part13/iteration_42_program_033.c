```cpp
// This file contains deliberately erroneous C++ constructs designed to trigger
// specific error message generation paths in GCC's parser (parser.cc).
// Each section corresponds to one of the RT_* enumeration cases listed.

// RT_EXTERN: Linkage specification without 'extern' keyword
"C" { void f(); }

// RT_STATIC_ASSERT: Incomplete static assertion
static_assert;

// RT_DECLTYPE: 'decltype' without parentheses or expression
decltype x = 0;

// RT_OPERATOR: Operator overload without specifying which operator
struct MyClass {
    int operator(int);
};

// RT_CLASS: Class definition without a name
class : public Base { };

// RT_TEMPLATE: Template declaration without parameters
template int x;

// RT_NAMESPACE: Namespace without valid identifier
namespace = alias;

// RT_USING: 'using' declaration without name
using;

// RT_ASM: Inline assembly without proper syntax
asm;

// RT_TRY: Try block without catch handler
try { int x = 5; }
```
