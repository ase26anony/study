```cpp
// This program is designed to trigger specific parsing errors in GCC's parser.cc
// Each section targets a different RT_* token error in cp_parser_error function

// Valid includes to provide context
#include <iostream>

// 1. Trigger RT_EXTERN error: Missing language string in extern block
extern {  // Error: expected 'extern' with language specifier
    void missing_language_spec();
}

// 2. Trigger RT_STATIC_ASSERT error: Incomplete static_assert
template<typename T>
struct TriggerStaticAssert {
    static_assert;  // Error: expected 'static_assert' with arguments
};

// 3. Trigger RT_DECLTYPE error: Malformed decltype
template<typename T>
void triggerDecltype() {
    decltype x y;  // Error: expected 'decltype' with parentheses
}

// 4. Trigger RT_OPERATOR error: Invalid operator overload
struct TriggerOperator {
    operator+;  // Error: expected 'operator' with valid operator symbol
};

// 5. Trigger RT_CLASS error: Invalid class definition
class 123;  // Error: expected 'class' with valid identifier

// 6. Trigger RT_TEMPLATE error: Malformed template declaration
template int x;  // Error: expected 'template' with parameter list

// 7. Trigger RT_NAMESPACE error: Invalid namespace definition
namespace = std;  // Error: expected 'namespace' with identifier or block

// 8. Trigger RT_USING error: Invalid using declaration
using;  // Error: expected 'using' with namespace or declaration

// 9. Trigger RT_ASM error: Incomplete inline assembly
void triggerAsm() {
    asm;  // Error: expected 'asm' with assembly string
}

// 10. Trigger RT_TRY error: Invalid try block
void triggerTry() {
    try x = 5;  // Error: expected 'try' with compound statement
}

// 11. Additional attempts to ensure parser sees all errors
// Wrap some errors in template to delay diagnostics
template<typename T>
struct ErrorContainer {
    // Another static_assert error
    static_assert(1);  // Error: missing message argument
    
    // Another decltype error
    decltype(;  // Error: malformed decltype expression
};

// Try to trigger namespace error in another context
namespace MissingBrace {
    // No closing brace - will cause various errors

// Valid main function to ensure compilation attempt proceeds
int main() {
    std::cout << "This program contains intentional parsing errors\n";
    return 0;
}

// Note: The parser will encounter multiple errors and should traverse
// through the various case labels in cp_parser_error function.
// Some errors may be fatal and stop parsing early, but placing them
// in different contexts (global, template, struct) helps maximize coverage.
```
