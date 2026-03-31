#ifdef __cplusplus
#include <iostream>
#endif

volatile int flag = 0;

int main() {
    // Valid main function that compiles successfully
    #ifdef __cplusplus
    std::cout << "Parser test" << std::endl;
    #endif
    
    // Use volatile to control flow and prevent dead code elimination
    volatile int test_case = 0;
    
    // RT_EXTERN - C and C++ mode
    if (test_case == 1) {
        // Missing 'extern' in linkage specification
        "C" void missing_extern_func();  // Should expect 'extern'
    }
    
    // RT_STATIC_ASSERT - C and C++ mode  
    if (test_case == 2) {
        // Missing 'static_assert' keyword
        (1 == 1, "static assertion failed");  // Should expect 'static_assert'
    }
    
    #ifdef __cplusplus
    // C++ specific tokens
    
    // RT_DECLTYPE
    if (test_case == 3) {
        // Missing 'decltype' in trailing return type
        auto missing_decltype_func() -> etype(1 + 2);  // Should expect 'decltype'
    }
    
    // RT_OPERATOR
    if (test_case == 4) {
        class MyClass {};
        // Missing 'operator' in operator overload
        int +(MyClass a, MyClass b) { return 0; }  // Should expect 'operator'
    }
    
    // RT_CLASS
    if (test_case == 5) {
        // Missing 'class' in class definition
        MissingClassKeyword {  // Should expect 'class'
            int x;
        };
    }
    
    // RT_TEMPLATE
    if (test_case == 6) {
        // Missing 'template' keyword
        <typename T> void missing_template_func() {}  // Should expect 'template'
    }
    
    // RT_NAMESPACE
    if (test_case == 7) {
        // Missing 'namespace' keyword
        missing_namespace_keyword {  // Should expect 'namespace'
            int x;
        }
    }
    
    // RT_USING
    if (test_case == 8) {
        // Missing 'using' in using directive
        namespace std;  // Should expect 'using'
    }
    
    // RT_TRY
    if (test_case == 9) {
        // Missing 'try' in try-catch block
        {  // Should expect 'try'
            throw 1;
        } catch (...) {}
    }
    #endif
    
    // RT_ASM - C and C++ mode
    if (test_case == 10) {
        // Missing 'asm' in inline assembly
        volatile ("nop");  // Should expect 'asm'
    }
    
    // Test with preprocessor macros to affect parser state
    #define BAD_CLASS class
    #ifdef __cplusplus
    if (test_case == 11) {
        BAD_ MyType {};  // Macro expansion missing token
    }
    #endif
    
    // Test with attributes
    if (test_case == 12) {
        // Attribute before missing extern
        __attribute__((weak)) "C" void attr_func();  // Should expect 'extern'
    }
    
    #ifdef __cplusplus
    // Nested scope test for RT_CLASS
    if (test_case == 13) {
        namespace Outer {
            // Missing 'class' inside namespace
            InnerClass {  // Should expect 'class'
                int member;
            };
        }
    }
    
    // Template context for RT_OPERATOR
    if (test_case == 14) {
        template<typename T>
        T missing_operator(T a, T b) {  // Should expect 'operator'
            return a + b;
        }
    }
    #endif
    
    return 0;
}
