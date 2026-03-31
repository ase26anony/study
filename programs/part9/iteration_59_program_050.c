```cpp
// This file contains deliberately erroneous C++ constructs designed to trigger
// specific "expected" error messages in GCC's parser.cc (lines 34457-34486).
// Each section targets a different RT_* case.

// RT_EXTERN: Linkage specification without 'extern'
"C" { void f(); }  // Missing 'extern' before linkage string

// RT_STATIC_ASSERT: Incomplete static assertion
static_assert;      // Missing parentheses and condition

// RT_DECLTYPE: Invalid decltype usage
decltype x = 0;     // Missing parentheses around expression

// RT_OPERATOR: Incomplete operator overload
class MyClass {
    int operator(int);  // Missing operator symbol (e.g., '+', '()')
};

// RT_CLASS: Malformed class definition
class : public Base { };  // Missing class name after 'class'

// RT_TEMPLATE: Invalid template declaration
template int x;           // Missing template parameters <...>

// RT_NAMESPACE: Malformed namespace definition
namespace = alias;        // Missing namespace name

// RT_USING: Incomplete using declaration
using;                    // Missing namespace or identifier

// RT_ASM: Invalid inline assembly
asm;                      // Missing assembly string and parentheses

// RT_TRY: Malformed try-block
try int x = 5;            // Missing compound statement and catch
```
