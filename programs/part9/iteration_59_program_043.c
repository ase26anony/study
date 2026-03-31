```cpp
// This file contains deliberately erroneous C++ constructs designed to trigger
// specific "expected" error messages in GCC's parser.cc (lines 34457-34486).
// Compile with: g++ -fsyntax-only -std=c++17 this_file.cpp

// 1. Trigger RT_EXTERN error: linkage specification without 'extern'
"C" {
    void some_function();
}

// 2. Trigger RT_STATIC_ASSERT error: incomplete static assertion
static_assert;

// 3. Trigger RT_DECLTYPE error: decltype without parentheses
decltype x = 0;

// 4. Trigger RT_OPERATOR error: operator without specifying which operator
class MyClass {
    int operator(int);
};

// 5. Trigger RT_CLASS error: class definition without name
class : public Base {
    int member;
};

// 6. Trigger RT_TEMPLATE error: template without parameters
template int y;

// 7. Trigger RT_NAMESPACE error: namespace without valid identifier
namespace 123 {
    int value;
}

// 8. Trigger RT_USING error: using declaration without name
using;

// 9. Trigger RT_ASM error: asm without assembly string
asm;

// 10. Trigger RT_TRY error: try without proper compound statement
try int z = 5;
```
