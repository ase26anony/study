#ifdef __cplusplus
#include <iostream>
#endif

volatile int flag = 0;

int main() {
    // Valid main function that compiles successfully
    #ifdef __cplusplus
    std::cout << "Parser test" << std::endl;
    #endif
    
    // Use volatile to prevent dead code elimination
    volatile int test_case = 0;
    
    // RT_EXTERN - C and C++ mode
    if (test_case == 1) {
        // Missing 'extern' in linkage specification
        "C" void missing_extern_func();  // Error: expected 'extern'
    }
    
    // RT_STATIC_ASSERT - C11 and C++ mode
    if (test_case == 2) {
        // Missing 'static_assert' keyword
        (1 == 1, "static assertion failed");  // Error: expected 'static_assert'
    }
    
    #ifdef __cplusplus
    // C++-specific tokens
    
    // RT_DECLTYPE
    if (test_case == 3) {
        // Missing 'decltype' in trailing return type
        auto missing_decltype_func() -> (x);  // Error: expected 'decltype'
    }
    
    // RT_OPERATOR  
    if (test_case == 4) {
        struct MyClass {
            int value;
        };
        // Missing 'operator' in overload
        int +(MyClass a, MyClass b) {  // Error: expected 'operator'
            return a.value + b.value;
        }
    }
    
    // RT_CLASS
    if (test_case == 5) {
        // Missing 'class' in class definition
        MissingClassKeyword {  // Error: expected 'class'
            int x;
        };
    }
    
    // RT_TEMPLATE
    if (test_case == 6) {
        // Missing 'template' keyword
        <typename T> void missing_template_func() {}  // Error: expected 'template'
    }
    
    // RT_NAMESPACE
    if (test_case == 7) {
        // Missing 'namespace' keyword
        MissingNamespace {  // Error: expected 'namespace'
            int x;
        }
    }
    
    // RT_USING
    if (test_case == 8) {
        // Missing 'using' in directive
        namespace std;  // Error: expected 'using'
    }
    
    // RT_TRY
    if (test_case == 9) {
        // Missing 'try' keyword
        {  // Error: expected 'try'
            throw 1;
        } catch (...) {}
    }
    #endif
    
    // RT_ASM - C and C++ mode
    if (test_case == 10) {
        // Missing 'asm' keyword
        volatile ("nop");  // Error: expected 'asm'
    }
    
    // Additional tests with preprocessor and attributes
    
    // Test with macro expansion
    #define BAD_CLASS class
    if (test_case == 11) {
        BAD_ MyType {};  // Error after macro expansion
    }
    
    // Test with __attribute__
    if (test_case == 12) {
        void __attribute__((noreturn)) missing_extern_func2();  // Error in attribute context
    }
    
    // Nested scope test
    #ifdef __cplusplus
    if (test_case == 13) {
        namespace Outer {
            // Missing 'class' inside namespace
            InnerClass {  // Error: expected 'class'
                int x;
            };
        }
    }
    #endif
    
    return 0;
}
