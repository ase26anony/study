```cpp
// This file contains deliberately erroneous C++ constructs designed to trigger
// specific "expected keyword" error messages in GCC's parser.
// Each section targets a different RT_* enumeration case from parser.cc.

// Target RT_EXTERN: Linkage specification without 'extern'
"C" { void f(); }  // Missing 'extern' before linkage string

// Target RT_STATIC_ASSERT: Incomplete static assertion
static_assert;      // Missing parentheses and condition

// Target RT_DECLTYPE: Invalid decltype usage
decltype x = 0;     // Missing parentheses around expression

// Target RT_OPERATOR: Incomplete operator overload
class MyClass {
    int operator(int);  // Missing operator token (e.g., '+', '()')
};

// Target RT_CLASS: Malformed class definition
class : public Base { };  // Missing class name after 'class'

// Target RT_TEMPLATE: Invalid template declaration
template int x;     // Missing template parameter list <...>

// Target RT_NAMESPACE: Invalid namespace definition
namespace = alias;  // Missing namespace name

// Target RT_USING: Incomplete using declaration
using;              // Missing namespace or identifier

// Target RT_ASM: Malformed inline assembly
asm;                // Missing parentheses and assembly string

// Target RT_TRY: Incomplete try-block
try int x = 5;      // Missing compound statement and catch handlers
```
