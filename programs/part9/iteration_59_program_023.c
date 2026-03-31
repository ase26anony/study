```cpp
// This file contains deliberate syntax errors to trigger specific
// "expected" error messages in GCC's parser.cc

// 1. Trigger RT_EXTERN error
// Linkage specification without 'extern'
"C" {
    void f();
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

// Anonymous struct in namespace scope (should also trigger)
struct {
    int x;
};

// 6. Trigger RT_TEMPLATE error
// Template without parameters
template int x;
template { };

// 7. Trigger RT_NAMESPACE error
// Namespace without proper identifier
namespace = alias;
namespace 123 {
    int y;
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
try { }
// Missing catch block

// Additional context to help parser differentiate some cases
void function() {
    // Try block in function scope
    try {
        int z = 0;
    }
    // Still missing catch here
}

// Template with decltype in invalid context
template<decltype>
void invalid_template() {}

// Operator in template context
template<typename T>
T operator(T a, T b) {
    return a + b;
}
```
