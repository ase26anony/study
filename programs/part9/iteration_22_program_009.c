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
    // Missing 'extern' in linkage specification
    if (selector == 1) {
        "C" void missing_extern_func();  // Error: expected 'extern'
    }
    
    // RT_STATIC_ASSERT - C11 and C++ mode
    #ifdef __cplusplus
    if (selector == 2) {
        static_assert (1, "fail");  // Correct - no error
        (1, "fail");  // Error: expected 'static_assert'
    }
    #endif
    
    // C mode static_assert test
    #if !defined(__cplusplus) && __STDC_VERSION__ >= 201112L
    if (selector == 3) {
        _Static_assert (1, "fail");  // Correct - no error  
        (1, "fail");  // Error: expected '_Static_assert' or 'static_assert'
    }
    #endif
    
    // RT_DECLTYPE - C++ only
    #ifdef __cplusplus
    if (selector == 4) {
        auto missing_decltype_func() -> etype(x);  // Error: expected 'decltype'
    }
    
    // RT_OPERATOR - C++ only
    // Missing 'operator' in operator overload
    class MyClass {};
    if (selector == 5) {
        int +(MyClass a, MyClass b);  // Error: expected 'operator'
    }
    
    // RT_CLASS - C++ only
    // Missing 'class' in class definition
    if (selector == 6) {
        MyMissingClass { public: int x; };  // Error: expected 'class'
    }
    
    // RT_TEMPLATE - C++ only  
    // Missing 'template' keyword
    if (selector == 7) {
        <typename T> void missing_template_func() {}  // Error: expected 'template'
    }
    
    // RT_NAMESPACE - C++ only
    // Missing 'namespace' keyword
    if (selector == 8) {
        my_ns { int x; };  // Error: expected 'namespace'
    }
    
    // RT_USING - C++ only
    // Missing 'using' in using directive
    if (selector == 9) {
        namespace std;  // Error: expected 'using'
    }
    
    // RT_TRY - C++ only
    // Missing 'try' in try-catch block
    if (selector == 10) {
        { throw 1; } catch (...) {}  // Error: expected 'try'
    }
    #endif
    
    // RT_ASM - C and C++ mode
    // Missing 'asm' in inline assembly
    if (selector == 11) {
        volatile ("nop");  // Error: expected 'asm'
    }
    
    // Test with preprocessor macros
    #ifdef __cplusplus
    #define BAD_CLASS class
    if (selector == 12) {
        BAD_ MyMacroType {};  // Error after macro expansion
    }
    #endif
    
    // Test with attributes
    #ifdef __cplusplus
    if (selector == 13) {
        __attribute__((unused)) MyAttrClass { int x; };  // Error after attribute
    }
    #endif
    
    // Nested scope test for class
    #ifdef __cplusplus
    namespace Outer {
        if (selector == 14) {
            Inner { int y; };  // Error in namespace scope
        }
    }
    #endif
    
    return 0;
}
