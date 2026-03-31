#ifdef __cplusplus
#include <iostream>
#endif

volatile int flag = 0;

int main() {
    // Valid main function that compiles
    #ifdef __cplusplus
    std::cout << "Parser test" << std::endl;
    #endif
    
    // Use volatile to control flow but ensure parser sees all code
    volatile int selector = 0;
    
    // RT_EXTERN - C and C++ mode
    if (selector == 1) {
        // Missing 'extern' in linkage specification
        "C" void missing_extern_func();  // Error: expected 'extern'
    }
    
    // RT_STATIC_ASSERT - C11 and C++11 onwards
    if (selector == 2) {
        // Missing 'static_assert' keyword
        (1 == 1, "static assertion failed");  // Error: expected 'static_assert'
    }
    
    #ifdef __cplusplus
    // C++ specific tokens
    
    // RT_DECLTYPE
    if (selector == 3) {
        // Missing 'decltype' in trailing return type
        auto missing_decltype_func() -> etype(5 + 3);  // Error: expected 'decltype'
    }
    
    // RT_OPERATOR
    if (selector == 4) {
        struct MyClass {
            int value;
        };
        // Missing 'operator' in overload
        int +(MyClass a, MyClass b) {  // Error: expected 'operator'
            return a.value + b.value;
        }
    }
    
    // RT_CLASS
    if (selector == 5) {
        // Missing 'class' keyword
        MissingClassKeyword {  // Error: expected 'class'
            int x;
        };
    }
    
    // RT_TEMPLATE
    if (selector == 6) {
        // Missing 'template' keyword
        <typename T>  // Error: expected 'template'
        void template_missing_func() {}
    }
    
    // RT_NAMESPACE
    if (selector == 7) {
        // Missing 'namespace' keyword
        my_namespace {  // Error: expected 'namespace'
            int value;
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
            throw 42;
        } catch (...) {
            // handle
        }
    }
    #endif
    
    // RT_ASM - C and C++ mode
    if (selector == 10) {
        // Missing 'asm' keyword
        volatile ("nop");  // Error: expected 'asm'
    }
    
    // Additional tests with preprocessor and attributes
    
    // Test with macro expansion
    #define BAD_CLASS class
    if (selector == 11) {
        BAD_ MyType {};  // Error after macro expansion
    }
    
    // Test with __attribute__
    if (selector == 12) {
        void __attribute__((noreturn)) missing_extern_func2();  // Error: expected 'extern'
    }
    
    // Nested in function
    if (selector == 13) {
        void outer_func() {
            // Missing 'class' inside function
            InnerClass {  // Error: expected 'class'
                int y;
            };
        }
    }
    
    #ifdef __cplusplus
    // Nested in namespace
    if (selector == 14) {
        namespace outer {
            // Missing 'template' inside namespace
            <typename U>  // Error: expected 'template'
            void nested_func() {}
        }
    }
    
    // In class definition
    if (selector == 15) {
        class Container {
            // Missing 'operator' inside class
            int +(int other) {  // Error: expected 'operator'
                return value + other;
            }
            int value;
        };
    }
    #endif
    
    return 0;
}
