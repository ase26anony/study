#ifdef __cplusplus
#include <iostream>
#endif

volatile int flag = 0;

int main() {
    // Valid main function that compiles
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
    if (selector == 2) {
        // Missing 'static_assert' keyword
        #ifdef __cplusplus
        (1 == 1, "static assertion failed");  // Error: expected 'static_assert'
        #else
        _Static_assert(1 == 1, "fail");  // C11 version, but we want to trigger RT_STATIC_ASSERT
        // Actually for C we need to trigger the C++ parser path
        #endif
    }
    
    #ifdef __cplusplus
    // C++ specific tokens
    
    // RT_DECLTYPE - C++11
    if (selector == 3) {
        // Missing 'decltype' in trailing return type
        auto missing_decltype_func() -> (x);  // Error: expected 'decltype'
    }
    
    // RT_OPERATOR - C++
    if (selector == 4) {
        class MyClass {
            int value;
        };
        // Missing 'operator' in operator overload declaration
        int +(MyClass a, MyClass b) { return a.value + b.value; }  // Error: expected 'operator'
    }
    
    // RT_CLASS - C++
    if (selector == 5) {
        // Missing 'class' keyword in class definition
        MissingClassKeyword {  // Error: expected 'class'
            int x;
        };
    }
    
    // RT_TEMPLATE - C++
    if (selector == 6) {
        // Missing 'template' keyword
        <typename T> void missing_template_func() {}  // Error: expected 'template'
    }
    
    // RT_NAMESPACE - C++
    if (selector == 7) {
        // Missing 'namespace' keyword
        my_namespace {  // Error: expected 'namespace'
            int x;
        }
    }
    
    // RT_USING - C++
    if (selector == 8) {
        // Missing 'using' keyword in using directive
        namespace std;  // Error: expected 'using'
    }
    
    // RT_TRY - C++
    if (selector == 9) {
        // Missing 'try' keyword
        {  // Error: expected 'try'
            throw 1;
        } catch (...) {
        }
    }
    #endif
    
    // RT_ASM - C and C++ (GCC extension)
    if (selector == 10) {
        // Missing 'asm' keyword in inline assembly
        volatile ("nop");  // Error: expected 'asm'
    }
    
    // Test with preprocessor macros
    #define BAD_CLASS class
    #ifdef __cplusplus
    if (selector == 11) {
        BAD_ MyType {};  // Error after macro expansion, might expect 'class'
    }
    #endif
    
    // Test with __attribute__
    if (selector == 12) {
        #ifdef __cplusplus
        __attribute__((noreturn)) void missing_extern_func2();  // Might expect 'extern' in some contexts
        #else
        __attribute__((noreturn)) void missing_extern_func2();  // C version
        #endif
    }
    
    // Nested in function for scope testing
    #ifdef __cplusplus
    if (selector == 13) {
        namespace test_ns {
            // Missing 'class' inside namespace
            NestedClass {  // Error: expected 'class'
                int y;
            };
        }
    }
    #endif
    
    return 0;
}

// Additional test cases outside main to ensure parser sees them
#ifdef __cplusplus
// RT_TEMPLATE in class context
template<typename T>
class TestTemplate {
    // Missing 'template' in member template
    <typename U> void member_func() {}  // Error: expected 'template'
};

// RT_OPERATOR in class context
class TestOperator {
    // Missing 'operator' in conversion operator
    int() const { return 42; }  // Error: expected 'operator'
};
#endif

// RT_EXTERN in C mode (compile with -xc)
#ifndef __cplusplus
// Missing 'extern' in C
"C" int missing_extern_var;  // Error: expected 'extern'
#endif
