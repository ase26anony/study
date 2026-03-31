```cpp
// This program is designed to trigger specific parsing errors in GCC's C++ parser
// Each section targets a different case in cp_parser_error function

// 1. Trigger RT_EXTERN error
// Invalid linkage specifier - missing language string
extern { void f1(); }  // Error: expected 'extern "C"' or similar

// 2. Trigger RT_STATIC_ASSERT error  
// Incomplete static_assert declaration
static_assert;  // Error: expected static_assert with arguments

// 3. Trigger RT_DECLTYPE error
// Malformed decltype usage
decltype x y;  // Error: expected decltype with parentheses

// 4. Trigger RT_OPERATOR error
// Invalid operator overload declaration
operator+;  // Error: expected complete operator declaration

// 5. Trigger RT_CLASS error
// Invalid class definition
class 123;  // Error: expected identifier after 'class'

// 6. Trigger RT_TEMPLATE error
// Malformed template declaration
template int x;  // Error: expected template parameter list

// 7. Trigger RT_NAMESPACE error
// Invalid namespace definition
namespace = foo;  // Error: expected namespace name or '{'

// 8. Trigger RT_USING error
// Invalid using declaration
using;  // Error: expected identifier after 'using'

// 9. Trigger RT_ASM error
// Incomplete inline assembly
asm;  // Error: expected '(' after 'asm'

// 10. Trigger RT_TRY error
// Invalid try block (in global scope to trigger during parsing)
// Note: try blocks can't be at global scope, so we'll create a malformed one
// inside a function template to delay the error
template<typename T>
void trigger_try_error() {
    try x = 5;  // Error: expected '{' after 'try'
}

// Main function that is syntactically correct
// This ensures the parser processes the entire file
int main() {
    return 0;
}

// Additional constructs to ensure parser sees all errors
// even if some are skipped due to earlier fatal errors

// Put some errors in template to delay parsing
template<typename T>
struct ErrorContainer {
    // Another static_assert error
    static_assert(1);  // Error: missing second argument and semicolon in wrong place
    
    // Another decltype error  
    decltype(;  // Error: malformed decltype
    
    // Another operator error
    operator  // Error: incomplete operator
};

// Namespace with errors
namespace BadNamespace {
    // Another extern error
    extern { int y; }  // Error: missing language string
    
    // Another template error
    template <>  // Error: missing declaration after template<>
}

// Using directive error in different context
struct S {
    using  // Error: expected identifier
};

// Try to trigger asm error in another context
void func() {
    asm  // Error: expected '('
}

// Final class error in template context
template<typename T>
class  // Error: expected identifier
```
