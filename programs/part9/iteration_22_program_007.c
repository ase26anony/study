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
    volatile int selector = 0;
    
    // RT_EXTERN - C and C++ mode
    if (selector == 1) {
        // Missing 'extern' in linkage specification
        "C" void missing_extern_func();  // Error: expected 'extern'
    }
    
    // RT_STATIC_ASSERT - C11 and C++11 onwards
    #if defined(__STDC_VERSION__) || defined(__cplusplus)
    if (selector == 2) {
        // Missing 'static_assert' keyword
        (1 == 1, "Static assertion failed");  // Error: expected 'static_assert'
    }
    #endif
    
    #ifdef __cplusplus
    // RT_DECLTYPE - C++ only
    if (selector == 3) {
        // Missing 'decltype' in trailing return type
        auto decltype_test() -> etype(x);  // Error: expected 'decltype'
    }
    
    // RT_OPERATOR - C++ only
    if (selector == 4) {
        struct MyClass {
            int value;
        };
        // Missing 'operator' in overload
        int +(MyClass a, MyClass b) {  // Error: expected 'operator'
            return a.value + b.value;
        }
    }
    
    // RT_CLASS - C++ only
    if (selector == 5) {
        // Missing 'class' keyword
        MyMissingClass {  // Error: expected 'class'
            int x;
        };
    }
    
    // RT_TEMPLATE - C++ only
    if (selector == 6) {
        // Missing 'template' keyword
        <typename T> void template_func() {}  // Error: expected 'template'
    }
    
    // RT_NAMESPACE - C++ only
    if (selector == 7) {
        // Missing 'namespace' keyword
        my_namespace {  // Error: expected 'namespace'
            int x;
        }
    }
    
    // RT_USING - C++ only
    if (selector == 8) {
        // Missing 'using' keyword
        namespace std;  // Error: expected 'using'
    }
    
    // RT_TRY - C++ only
    if (selector == 9) {
        // Missing 'try' keyword
        {  // Error: expected 'try'
            throw 1;
        } catch (...) {
        }
    }
    #endif  // __cplusplus
    
    // RT_ASM - C and C++ mode (GCC extension)
    if (selector == 10) {
        // Missing 'asm' keyword
        volatile ("nop");  // Error: expected 'asm'
    }
    
    // Test with preprocessor macros to affect parser state
    #define BAD_CLASS class
    #ifdef __cplusplus
    if (selector == 11) {
        BAD_ MyType {};  // Error after macro expansion
    }
    #endif
    
    // Test with attributes
    if (selector == 12) {
        void __attribute__((noreturn)) missing_extern_func2();  // Missing 'extern'
    }
    
    // Nested scope tests
    #ifdef __cplusplus
    if (selector == 13) {
        namespace outer {
            // Missing 'namespace' in nested context
            inner {  // Error: expected 'namespace'
                int y;
            }
        }
    }
    
    if (selector == 14) {
        class Container {
            // Missing 'template' in member context
            <typename U> void member_func();  // Error: expected 'template'
        };
    }
    #endif
    
    return 0;
}
