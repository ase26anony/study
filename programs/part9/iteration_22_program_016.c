#ifdef __cplusplus
#include <iostream>
#endif

volatile int parser_trigger = 0;

int main() {
    // Valid main function that compiles successfully
    #ifdef __cplusplus
    std::cout << "Parser test program" << std::endl;
    #endif
    
    // Use volatile to prevent dead code elimination
    if (parser_trigger) {
        // This block won't execute, but will be parsed
        
        /* 1. RT_EXTERN: Missing 'extern' in linkage specification */
        "C" void missing_extern_func();  // Error: expected 'extern'
        
        /* 2. RT_STATIC_ASSERT: Missing 'static_assert' keyword */
        #ifdef __cplusplus
        (1 == 1, "static_assert without keyword");  // Error in C++ mode
        #else
        _Static_assert(1 == 1, "C static assert");  // Valid in C, test different path
        #endif
    }
    
    if (parser_trigger + 1) {
        /* 3. RT_DECLTYPE: Misspelled decltype in trailing return (C++ only) */
        #ifdef __cplusplus
        auto decltype_error() -> etype(parser_trigger);  // Error: expected 'decltype'
        #endif
        
        /* 4. RT_OPERATOR: Missing 'operator' in overload definition */
        #ifdef __cplusplus
        class MyClass {};
        int +(MyClass a, MyClass b) { return 0; }  // Error: expected 'operator'
        #endif
    }
    
    // Macro expansion to test parser state machine
    #define BAD_CLASS class
    #define BAD_TEMPLATE template
    
    if (parser_trigger + 2) {
        /* 5. RT_CLASS: Missing 'class' in class definition */
        #ifdef __cplusplus
        BAD_ MissingClass {  // Error after macro: expected 'class'
            int x;
        };
        #endif
        
        /* 6. RT_TEMPLATE: Missing 'template' keyword */
        #ifdef __cplusplus
        <typename T> void template_error() {}  // Error: expected 'template'
        #endif
    }
    
    // Test with __attribute__ specifiers
    if (parser_trigger + 3) __attribute__((unused)) {
        /* 7. RT_NAMESPACE: Missing 'namespace' keyword */
        #ifdef __cplusplus
        my_namespace {  // Error: expected 'namespace'
            int y;
        }
        #endif
        
        /* 8. RT_USING: Missing 'using' in directive */
        #ifdef __cplusplus
        namespace std;  // Error: expected 'using'
        #endif
    }
    
    // Nested in function-like context
    void nested_errors() {
        if (parser_trigger + 4) {
            /* 9. RT_ASM: Missing 'asm' in inline assembly */
            #ifdef __cplusplus
            volatile ("nop");  // Error in C++: expected 'asm'
            #else
            // In C, test different asm syntax
            __asm__ volatile ("nop");
            #endif
            
            /* 10. RT_TRY: Missing 'try' in try-catch block (C++ only) */
            #ifdef __cplusplus
            {  // Error: expected 'try'
                throw 42;
            } catch (...) {
                // handle
            }
            #endif
        }
    }
    
    // Additional C-specific tests
    #ifndef __cplusplus
    if (parser_trigger + 5) {
        // C mode tests for tokens that exist in C
        extern int c_var;
        
        // Test asm in C mode
        asm volatile ("nop");
        
        // Test _Static_assert error case
        _Static_assert(1, "valid");
    }
    #endif
    
    // Test static_assert in C++ mode with error
    #ifdef __cplusplus
    if (parser_trigger + 6) {
        // Another static_assert error case
        static_assert(1 == 1);  // Missing message string might trigger different path
    }
    #endif
    
    // Complex nested scenario
    #ifdef __cplusplus
    namespace outer {
        class Container {
            if (parser_trigger + 7) {
                // Template with missing 'template' inside class
                <class U> void member_func() {}  // Error: expected 'template'
            }
        };
    }
    #endif
    
    return 0;
}
