#ifdef __cplusplus
#include <iostream>
#endif

volatile int flag = 0;

int main() {
    // Main function is valid and compilable
    volatile int selector = 0;
    
    // Each error block is isolated to prevent cascading failures
    // Only one block will be parsed at a time based on volatile conditions
    
    // RT_EXTERN error - C and C++ mode
    if (selector == 1) {
        // Missing 'extern' in linkage specification
        "C" void missing_extern_func();  // Error: expected 'extern'
    }
    
    // RT_STATIC_ASSERT error - C11/C++11 and later
    if (selector == 2) {
        // Missing 'static_assert' keyword
        #ifdef __cplusplus
        (1 == 1, "static assertion failed");  // Error: expected 'static_assert'
        #else
        _Static_assert(1 == 1, "fail");  // C11 version
        #endif
    }
    
#ifdef __cplusplus
    // C++-specific tokens
    
    // RT_DECLTYPE error
    if (selector == 3) {
        // Missing 'decltype' in trailing return type
        auto missing_decltype_func() -> etype(x) { return x; }  // Error: expected 'decltype'
    }
    
    // RT_OPERATOR error
    if (selector == 4) {
        struct MyClass {
            int value;
        };
        // Missing 'operator' in operator overload
        int +(MyClass a, MyClass b) {  // Error: expected 'operator'
            return a.value + b.value;
        }
    }
    
    // RT_CLASS error - with macro expansion
    if (selector == 5) {
        #define BAD_CLASS class
        BAD_ MissingClass {  // Error: expected 'class' (after macro expansion)
            int x;
        };
    }
    
    // RT_TEMPLATE error - inside function
    if (selector == 6) {
        void template_error_func() {
            // Missing 'template' keyword
            <typename T> void inner_func() {}  // Error: expected 'template'
        }
    }
    
    // RT_NAMESPACE error - with attribute
    if (selector == 7) {
        __attribute__((deprecated))
        missing_namespace {  // Error: expected 'namespace'
            int x;
        }
    }
    
    // RT_USING error
    if (selector == 8) {
        // Missing 'using' in using directive
        namespace std;  // Error: expected 'using'
    }
    
    // RT_ASM error - C++ mode
    if (selector == 9) {
        // Missing 'asm' keyword
        volatile ("nop");  // Error: expected 'asm'
    }
    
    // RT_TRY error - nested in control flow
    if (selector == 10) {
        void try_block_test() {
            volatile int should_try = 0;
            if (should_try) {
                // Missing 'try' keyword
                {  // Error: expected 'try'
                    throw 1;
                } catch (...) {
                    // handle
                }
            }
        }
    }
#endif

    // RT_ASM error - C mode (outside C++ guard)
    if (selector == 11) {
        // Missing 'asm' in C inline assembly
        __asm__ volatile ("nop");  // Valid for comparison
        volatile ("nop");  // Error: expected 'asm' or '__asm__'
    }
    
    // RT_EXTERN error - C mode with attribute
    if (selector == 12) {
        __attribute__((weak))
        "C" int missing_extern_var;  // Error: expected 'extern'
    }
    
    // RT_STATIC_ASSERT error - in declaration context
    if (selector == 13) {
        #ifdef __cplusplus
        struct WithAssert {
            // Missing 'static_assert' in class scope
            (sizeof(int) == 4, "int must be 4 bytes");  // Error: expected 'static_assert'
        };
        #endif
    }
    
    return 0;
}

// Additional error contexts in different scopes
#ifdef __cplusplus
namespace Outer {
    // RT_CLASS error at namespace scope
    class OuterClass {
        // RT_OPERATOR error inside class
        int operator()(int x) { return x; }  // Valid
        missing_operator +(int x) { return x; }  // Error: expected 'operator'
    };
    
    // RT_TEMPLATE error with nested template
    template<typename T>
    struct TemplateStruct {
        // Missing 'template' for member template
        <typename U> void member_func() {}  // Error: expected 'template'
    };
}
#endif

// RT_EXTERN error at file scope (C and C++)
"C" int file_scope_var = 0;  // Error: expected 'extern'

// RT_STATIC_ASSERT at file scope (C11/C++11)
#ifdef __cplusplus
static_assert(1 == 1, "valid");
missing_static_assert(1 == 1, "invalid");  // Error: expected 'static_assert'
#else
_Static_assert(1 == 1, "valid C");
missing__Static_assert(1 == 1, "invalid");  // Error
#endif
