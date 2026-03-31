```cpp
// This file contains deliberately erroneous C++ constructs designed to trigger
// specific parser error messages in GCC's parser.cc (lines 34457-34486).
// Each section targets one RT_* case. Compile with: g++ -fsyntax-only -std=c++17 this_file.cpp

// 1. Trigger RT_EXTERN error: missing 'extern' before linkage specification
"C" { void f(); }  // Error: expected 'extern'

// 2. Trigger RT_STATIC_ASSERT error: incomplete static assertion
static_assert;      // Error: expected 'static_assert' (proper syntax)

// 3. Trigger RT_DECLTYPE error: decltype without parentheses
decltype x = 0;     // Error: expected 'decltype' (with expression)

// 4. Trigger RT_OPERATOR error: operator without specifying which operator
struct MyClass {
    int operator(int);  // Error: expected 'operator' (followed by operator token)
};

// 5. Trigger RT_CLASS error: class definition without name
class : public Base { };  // Error: expected 'class' (identifier after keyword)

// 6. Trigger RT_TEMPLATE error: template without parameters
template int x;     // Error: expected 'template' (parameter list)

// 7. Trigger RT_NAMESPACE error: namespace without valid identifier
namespace = alias;  // Error: expected 'namespace' (identifier)

// 8. Trigger RT_USING error: using without namespace/identifier
using;              // Error: expected 'using' (namespace or declaration)

// 9. Trigger RT_ASM error: asm without string literal
asm;                // Error: expected 'asm' (assembly template)

// 10. Trigger RT_TRY error: try without proper block/catch
try int x = 5;      // Error: expected 'try' (compound statement)
```
