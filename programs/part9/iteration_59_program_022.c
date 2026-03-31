```cpp
// Coverage test for parser.cc error messages
// This file contains deliberate syntax errors to trigger specific
// "expected" error messages in GCC's parser

// 1. Trigger RT_EXTERN error
// Linkage specification without 'extern' keyword
"C" {
    void f1();
}

// 2. Trigger RT_STATIC_ASSERT error
// Incomplete static assertion
static_assert;
static_assert "message";

// 3. Trigger RT_DECLTYPE error
// decltype without proper parentheses
decltype x = 0;
template<typename T>
typename decltype;

// 4. Trigger RT_OPERATOR error
// Operator overload without specifying which operator
class MyClass {
    int operator(int);
    MyClass::operator { return 0; }
};

// 5. Trigger RT_CLASS error
// Class definition without name
class : public Base {
    int x;
};

// Anonymous struct in namespace scope (also problematic)
struct {
    int y;
};

// 6. Trigger RT_TEMPLATE error
// Template without parameters
template int x;
template { }

// 7. Trigger RT_NAMESPACE error
// Namespace without valid name
namespace = alias;
namespace 123 {
    int z;
}

// 8. Trigger RT_USING error
// Using declaration/directive without proper syntax
using;
using namespace = std;

// 9. Trigger RT_ASM error
// Inline assembly without proper syntax
asm;
asm 123;

// 10. Trigger RT_TRY error
// Try block without proper structure
try int x = 5;
try {
    // No catch handler
}

// Additional invalid constructs to ensure parser attempts all cases
class AnotherClass {
    // Invalid operator syntax
    operator;
    
    // Invalid template member
    template member;
};

// Invalid linkage specification continuation
{
    void g();
}

// Invalid decltype in return type
decltype auto func();

// Note: This file will not compile successfully
// It's designed specifically to trigger parser error paths
```
