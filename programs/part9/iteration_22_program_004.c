#ifdef __cplusplus
#include <iostream>
#endif

volatile int flag = 0;

int main() {
    // Valid main function that will compile
    #ifdef __cplusplus
    std::cout << "Parser test" << std::endl;
    #endif
    
    // Use volatile to prevent dead code elimination
    volatile int selector = 0;
    
    // RT_EXTERN - C and C++ mode
    if (selector == 1) {
        // Missing 'extern' in linkage specification
        "C" void missing_extern_func();  // Error: expected 'extern'
    }
    
    // RT_STATIC_ASSERT - C11/C++11 and later
    #if defined(__STDC_VERSION__) || defined(__cplusplus)
    if (selector == 2) {
        // Missing 'static_assert' keyword
        (1 == 1, "Static assertion failed");  // Error: expected 'static_assert'
    }
    #endif
    
    #ifdef __cplusplus
    // C++-specific tokens
    
    // RT_DECLTYPE
    if (selector == 3) {
        // Missing 'decltype' in trailing return type
        auto missing_decltype_func() -> etype(x) { return 0; }  // Error: expected 'decltype'
    }
    
    // RT_OPERATOR  
    if (selector == 4) {
        struct MyClass {
            int value;
        };
        // Missing 'operator' in operator overload
        int +(MyClass a, MyClass b) { return a.value + b.value; }  // Error: expected 'operator'
    }
    
    // RT_CLASS
    if (selector == 5) {
        // Missing 'class' keyword in class definition
        MissingClassKeyword {  // Error: expected 'class'
            int x;
        };
    }
    
    // RT_TEMPLATE
    if (selector == 6) {
        // Missing 'template' keyword
        <typename T> void missing_template_func() {}  // Error: expected 'template'
    }
    
    // RT_NAMESPACE
    if (selector == 7) {
        // Missing 'namespace' keyword
        missing_namespace {  // Error: expected 'namespace'
            int x;
        }
    }
    
    // RT_USING
    if (selector == 8) {
        // Missing 'using' keyword
        namespace std;  // Error: expected 'using'
    }
    
    // RT_TRY
    if (selector == 9) {
        // Missing 'try' keyword
        {  // Error: expected 'try'
            throw 1;
        } catch (...) {
        }
    }
    #endif  // __cplusplus
    
    // RT_ASM - C and C++ mode
    if (selector == 10) {
        // Missing 'asm' keyword for inline assembly
        volatile ("nop");  // Error: expected 'asm'
    }
    
    // Additional tests with preprocessor and attributes
    
    // Test with macro expansion
    #define BAD_CLASS class
    if (selector == 11) {
        BAD_ MyType {};  // Error after macro expansion
    }
    
    // Test with __attribute__
    #ifdef __GNUC__
    if (selector == 12) {
        void __attribute__((noreturn)) missing_extern_func2();  // Error in attribute context
    }
    #endif
    
    // Nested scope test
    if (selector == 13) {
        #ifdef __cplusplus
        namespace outer {
            // Missing 'namespace' inside another namespace
            inner {  // Error: expected 'namespace'
                int x;
            }
        }
        #endif
    }
    
    // Multiple errors in different conditional blocks
    // Each is isolated by being in a separate if block
    // that the parser must parse but won't execute
    
    return 0;
}
