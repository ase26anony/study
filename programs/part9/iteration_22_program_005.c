#ifdef __cplusplus
#include <iostream>
#endif

volatile int flag = 0;

int main() {
    // Main function is valid and compilable
    volatile int mode = 0;
    
    // Each error block is isolated to prevent cascading failures
    // Only one block will be parsed at a time based on volatile conditions
    
    // RT_EXTERN error - C and C++ mode
    if (mode == 1) {
        // Missing 'extern' in linkage specification
        "C" void missing_extern_func();  // Error: expected 'extern'
    }
    
    // RT_STATIC_ASSERT error - C11 and C++11 onwards
    #ifdef __cplusplus
    if (mode == 2) {
        // Missing 'static_assert' keyword
        (1 == 1, "static assertion failed");  // Error: expected 'static_assert'
    }
    #else
    if (mode == 2) {
        // C mode static_assert error
        _Static_assert  // Missing parentheses to trigger different error path
    }
    #endif
    
    #ifdef __cplusplus
    // C++ specific tokens
    
    // RT_DECLTYPE error
    if (mode == 3) {
        // Missing 'decltype' in trailing return type
        auto decltype_error() -> etype(x) { return 0; }  // Error: expected 'decltype'
    }
    
    // RT_OPERATOR error
    if (mode == 4) {
        class MyClass {};
        // Missing 'operator' in operator overload
        int +(MyClass a, MyClass b) { return 0; }  // Error: expected 'operator'
    }
    
    // RT_CLASS error - inside macro expansion
    #define BAD_CLASS class
    if (mode == 5) {
        // Missing 'class' keyword
        BAD_ MissingClass {  // Error: expected 'class'
            int x;
        };
    }
    
    // RT_TEMPLATE error - with __attribute__
    if (mode == 6) {
        // Missing 'template' keyword
        __attribute__((unused)) <typename T>  // Error: expected 'template'
        void template_error() {}
    }
    
    // RT_NAMESPACE error - nested in function
    if (mode == 7) {
        // Missing 'namespace' keyword
        void namespace_error() {
            my_ns {  // Error: expected 'namespace'
                int x;
            }
        }
    }
    
    // RT_USING error - with volatile control flow
    if (mode == 8) {
        volatile int trigger = 0;
        if (trigger) {
            // Missing 'using' keyword
            namespace std;  // Error: expected 'using'
        }
    }
    
    // RT_TRY error - in appropriate context
    if (mode == 9) {
        // Missing 'try' keyword
        {  // Error: expected 'try'
            throw 1;
        } catch (...) {
        }
    }
    #endif  // __cplusplus
    
    // RT_ASM error - C and C++ mode, inside attribute
    if (mode == 10) {
        __attribute__((naked))
        void asm_error() {
            volatile ("nop");  // Error: expected 'asm'
        }
    }
    
    return 0;
}

// Additional error contexts in different scopes
#ifdef __cplusplus
namespace Outer {
    // RT_CLASS error in namespace scope
    class OuterClass {
        // RT_OPERATOR error in class scope
        int operator() { return 0; }  // Valid, but surrounding context matters
    };
    
    // RT_TEMPLATE error with macro
    #define PROBLEMATIC_TEMPLATE template
    PROBLEMATIC_ <int>  // Error: expected 'template'
    void another_error() {}
}
#endif

// RT_EXTERN error in global scope with C linkage
"C" int global_without_extern;  // Error: expected 'extern'

// RT_STATIC_ASSERT error at file scope
#ifdef __cplusplus
static_assert  // Missing parentheses and message
#else
_Static_assert  // Incomplete in C mode
#endif
