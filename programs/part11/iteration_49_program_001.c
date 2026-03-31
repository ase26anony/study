```cpp
// test_parser_errors.cc
// This file contains multiple syntax errors designed to trigger
// specific error-handling code in the C++ parser (lines 34457-34486 of parser.cc).
// Compile with: g++ -fsyntax-only -c test_parser_errors.cc 2>&1

// ============================================================================
// 1. Malformed Linkage Specifications (expecting 'extern')
// ============================================================================
extern ;                    // Error: missing language string
extern "C"                 // Error: no following declaration
extern 123 int x;          // Error: invalid language specifier

// ============================================================================
// 2. Broken static_assert Declarations (expecting 'static_assert')
// ============================================================================
static_assert;             // Error: missing condition and message
static_assert true;        // Error: missing parentheses
static_assert(1,);         // Error: missing message string

// ============================================================================
// 3. Incomplete decltype Expressions (expecting 'decltype')
// ============================================================================
decltype;                  // Error: no argument
decltype x;                // Error: missing parentheses around x
decltype(;                 // Error: unclosed parenthesis, empty expression

// ============================================================================
// 4. Misplaced operator Keyword (expecting 'operator')
// ============================================================================
int operator;              // Error: no operator symbol
operator+                  // Error: missing return type, not in class scope

struct S {
    operator ;             // Error: incomplete conversion function
};

// ============================================================================
// 5. Class/Struct Definition Errors (expecting 'class')
// ============================================================================
class ;                    // Error: no identifier
class S {                  // Error: no closing brace (EOF will trigger)
// Note: Actual EOF comes later in file

template<typename T> class // Error: no class name after keyword

// ============================================================================
// 6. Template Declaration Mistakes (expecting 'template')
// ============================================================================
template<typename T> ;     // Error: no declaration after >
template                   // Error: no <...> or declaration
template<>                 // Error: no specialization declaration

// ============================================================================
// 7. Namespace Errors (expecting 'namespace' or 'using')
// ============================================================================
namespace ;                // Error: no identifier
namespace X {              // Error: no closing brace (EOF will trigger)
using namespace ;          // Error: no namespace name

// ============================================================================
// 8. Incorrect asm Usage (expecting 'asm')
// ============================================================================
asm;                       // Error: no assembly template
asm volatile;              // Error: missing parentheses and template
asm("mov %0, %1" : : "r"(x)); // Error: output operand missing

// ============================================================================
// 9. Try-Block Syntax Errors (expecting 'try')
// ============================================================================
try ;                      // Error: no compound statement
try { }                    // Error: no catch handler
try                        // Error: EOF after keyword

// ============================================================================
// 10. Preprocessor-Directed Errors (for targeted testing)
// ============================================================================
#ifdef TEST_EXTERN
    extern "C++"           // No following declaration
#endif

#if TEST_CASE == 1
    static_assert(;        // Malformed
#elif TEST_CASE == 2
    decltype(              // Unclosed
#endif

// ============================================================================
// 11. Parser Recovery State Exploitation
// ============================================================================
// Cascading error: linkage spec inside block scope
{
    extern "C" int x;      // Valid but inside block - may cause recovery issues
    class                  // Parser may expect 'class' keyword after recovery
}

// Template inside class without member
class RecoveryTest {
    template<typename T>   // No member declaration after this
    // Parser may expect decltype, operator, etc. after seeing >
    decltype               // This might trigger in recovery
};

// Nested errors to confuse parser state
namespace Confusing {
    template<              // Unclosed template parameter list
    extern "C"             // Unexpected in this context
    static_assert(1        // Unclosed static_assert
}

// ============================================================================
// 12. Additional Context-Specific Errors
// ============================================================================
// At global scope, expecting various keywords
using                     // Incomplete using declaration

// In class scope
struct AnotherClass {
    operator              // Incomplete operator
    template              // Bare template keyword
    try                   // Try in invalid context
};

// In function scope (though function won't parse due to earlier errors)
void brokenFunction() {
    asm                   // Incomplete asm
    decltype(             // Unclosed decltype
}

// Note: This file intentionally does not have proper closing braces
// for some constructs to trigger EOF-related parser states.
```
