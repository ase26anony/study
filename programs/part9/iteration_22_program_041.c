#ifdef __cplusplus
#include <iostream>
#endif

volatile int parser_mode = 0;

int main() {
    // Valid main function that compiles successfully
    #ifdef __cplusplus
    std::cout << "Parser test" << std::endl;
    #endif
    
    // Use volatile to prevent dead code elimination
    volatile int trigger = 0;
    
    // ===== RT_EXTERN =====
    // C and C++: Missing 'extern' in linkage specification
    if (trigger == 1) {
        // Missing 'extern' before "C"
        "C" void missing_extern_func();  // Error: expected 'extern'
    }
    
    // ===== RT_STATIC_ASSERT =====  
    // C11/C++11: Missing 'static_assert' keyword
    #ifdef __cplusplus
    if (trigger == 2) {
        // Missing 'static_assert' keyword
        (1 == 1, "Should be true");  // Error: expected 'static_assert'
    }
    #else
    if (trigger == 2) {
        _Static_assert // Use C11's _Static_assert to trigger similar path
        (1 == 2, "Should fail");
    }
    #endif
    
    // ===== RT_DECLTYPE ===== (C++ only)
    #ifdef __cplusplus
    if (trigger == 3) {
        // Missing 'decltype' in trailing return type
        auto missing_decltype_func() -> etype(1 + 2) {  // Error: expected 'decltype'
            return 3;
        }
    }
    #endif
    
    // ===== RT_OPERATOR ===== (C++ only)
    #ifdef __cplusplus
    if (trigger == 4) {
        class MyClass {
            int value;
        public:
            MyClass(int v) : value(v) {}
        };
        
        // Missing 'operator' in operator overload
        int +(MyClass a, MyClass b) {  // Error: expected 'operator'
            return a.value + b.value;
        }
    }
    #endif
    
    // ===== RT_CLASS ===== (C++ only)
    #ifdef __cplusplus
    if (trigger == 5) {
        // Missing 'class' keyword in class definition
        MissingClassKeyword {  // Error: expected 'class'
            int x;
        };
    }
    #endif
    
    // ===== RT_TEMPLATE ===== (C++ only)
    #ifdef __cplusplus
    if (trigger == 6) {
        // Missing 'template' keyword
        <typename T>  // Error: expected 'template'
        void template_missing_func() {}
    }
    #endif
    
    // ===== RT_NAMESPACE ===== (C++ only)
    #ifdef __cplusplus
    if (trigger == 7) {
        // Missing 'namespace' keyword
        my_namespace {  // Error: expected 'namespace'
            int y;
        }
    }
    #endif
    
    // ===== RT_USING ===== (C++ only)
    #ifdef __cplusplus
    if (trigger == 8) {
        // Missing 'using' in using directive
        namespace std;  // Error: expected 'using'
    }
    #endif
    
    // ===== RT_ASM =====
    // C and C++: Missing 'asm' keyword
    if (trigger == 9) {
        // Missing 'asm' in inline assembly
        volatile ("nop");  // Error: expected 'asm'
    }
    
    // ===== RT_TRY ===== (C++ only)
    #ifdef __cplusplus
    if (trigger == 10) {
        // Missing 'try' keyword
        {  // Error: expected 'try'
            throw 42;
        } catch (...) {
            // Handle exception
        }
    }
    #endif
    
    // ===== Additional tests with preprocessor and attributes =====
    
    // Test with macro expansion
    #define BAD_CLASS class
    #ifdef __cplusplus
    if (trigger == 11) {
        BAD_ MyMacroClass {};  // Error after macro expansion
    }
    #endif
    
    // Test with __attribute__
    if (trigger == 12) {
        // Attribute followed by missing extern
        __attribute__((visibility("default")))
        "C" void attr_missing_extern();  // Error in attribute context
    }
    
    // Nested scope test
    #ifdef __cplusplus
    if (trigger == 13) {
        namespace outer {
            // Missing 'class' inside namespace
            InnerClassMissingKeyword {  // Error in namespace scope
                int z;
            };
        }
    }
    #endif
    
    return 0;
}
