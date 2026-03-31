```cpp
// This file contains intentionally invalid C++ code designed to trigger
// specific error message generation paths in GCC's parser.
// Each section corresponds to one of the RT_* cases in parser.cc.

// RT_EXTERN: Linkage specification without 'extern'
"C" { void f(); }

// RT_STATIC_ASSERT: Incomplete static assertion
static_assert;

// RT_DECLTYPE: decltype without proper parentheses
decltype x = 0;

// RT_OPERATOR: operator without specifying which operator
struct MyClass {
    int operator(int);
};

// RT_CLASS: Class definition without name
class : public Base { };

// RT_TEMPLATE: template without parameters
template int x;

// RT_NAMESPACE: namespace without valid identifier
namespace = alias;

// RT_USING: using without namespace or identifier
using;

// RT_ASM: asm without assembly string
asm;

// RT_TRY: try without proper catch block
try int x = 5;
```
