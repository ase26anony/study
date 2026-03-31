```cpp
// Coverage test for GCC parser error messages
// This file contains deliberate syntax errors to trigger specific
// "expected <keyword>" error messages in parser.cc

// 1. Trigger RT_EXTERN error
// Linkage specification without 'extern' keyword
"C" { void f(); }  // Error: expected 'extern'

// 2. Trigger RT_STATIC_ASSERT error  
// Incomplete static assertion
static_assert;  // Error: expected 'static_assert' with proper syntax

// 3. Trigger RT_DECLTYPE error
// decltype without parentheses or expression
decltype x = 0;  // Error: expected 'decltype(expression)'

// 4. Trigger RT_OPERATOR error
// Operator overload without operator token
class MyClass {
    int operator(int);  // Error: expected specific operator after 'operator'
};

// 5. Trigger RT_CLASS error
// Class definition without name
class : public Base { };  // Error: expected class name after 'class'

// 6. Trigger RT_TEMPLATE error
// Template declaration without parameters
template int x;  // Error: expected template parameter list

// 7. Trigger RT_NAMESPACE error
// Namespace without valid identifier
namespace 123 { }  // Error: expected namespace identifier

// 8. Trigger RT_USING error
// Using declaration without name
using;  // Error: expected namespace or qualified identifier

// 9. Trigger RT_ASM error
// Inline assembly without proper syntax
asm;  // Error: expected assembly template string

// 10. Trigger RT_TRY error
// Try block without catch handlers
try { 
    int x = 5; 
}  // Error: expected catch handlers after try block

// Additional test cases to ensure parser reaches all paths

// Another extern linkage test
"C++" { void g(); }  // Error: expected 'extern'

// Another static_assert test  
static_assert "message";  // Error: expected parentheses and condition

// Another decltype test in template context
template<typename T = decltype> struct S {};  // Error: expected expression

// Another operator test
struct Another {
    operator { return 0; }  // Error: expected operator token
};

// Another class test
struct { int x; };  // In context where named class is required

// Another template test
template { };  // Error: expected template parameters

// Another namespace test
namespace = alias;  // Error: expected namespace identifier

// Another using test  
using namespace = std;  // Error: expected namespace name

// Another asm test
asm 123;  // Error: expected string literal

// Another try test
try int y = 10;  // Error: expected compound statement
```
