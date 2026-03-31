```cpp
// This file contains deliberate syntax errors to trigger specific
// "expected <keyword>" error messages in GCC's parser.
// Compile with: g++ -fsyntax-only -std=c++17 this_file.cpp

// 1. Trigger RT_EXTERN error
// Linkage specification without 'extern' keyword
"C" { void f1(); }  // Expected: expected 'extern'

// 2. Trigger RT_STATIC_ASSERT error
// Incomplete static assertion
static_assert;  // Expected: expected 'static_assert' (proper syntax)

// 3. Trigger RT_DECLTYPE error
// decltype without parentheses in declaration
decltype x = 0;  // Expected: expected 'decltype' (proper syntax)

// 4. Trigger RT_OPERATOR error
// Operator overload without operator token
struct MyClass {
    int operator(int);  // Expected: expected 'operator' (specific operator)
};

// 5. Trigger RT_CLASS error
// Class definition without name but with base specifier
class : public Base { };  // Expected: expected 'class' (identifier)

// 6. Trigger RT_TEMPLATE error
// Template without parameters
template int x;  // Expected: expected 'template' (parameter list)

// 7. Trigger RT_NAMESPACE error
// Namespace with numeric name
namespace 123 { }  // Expected: expected 'namespace' (identifier)

// 8. Trigger RT_USING error
// Using declaration without identifier
using;  // Expected: expected 'using' (namespace or identifier)

// 9. Trigger RT_ASM error
// Inline assembly without string literal
asm;  // Expected: expected 'asm' (string literal)

// 10. Trigger RT_TRY error
// Try block without catch
try { int y = 5; }  // Expected: expected 'try' (catch handler)

// Additional invalid constructs to ensure parser attempts all errors

// Invalid template usage
template <>  // Incomplete template specialization

// Invalid operator in class context
class AnotherClass {
    operator { return 0; }  // Missing operator token
};

// Invalid using directive
using namespace = std;  // '=' not allowed

// Invalid asm with numeric argument
asm 123;

// Invalid decltype in template argument
template<typename decltype>  // decltype without parentheses
struct Invalid {};

// Note: The parser may stop after first major error,
// but these constructs are designed to trigger the specific
// error message generation paths before bailing out.
```
