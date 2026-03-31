#ifdef __cplusplus
#include <iostream>
#endif

volatile int flag = 0;

int main() {
    // Valid main function that compiles successfully
    #ifdef __cplusplus
    std::cout << "Parser test" << std::endl;
    #endif
    
    // Use volatile to control flow but ensure parser sees all code
    volatile int parser_test = 0;
    
    // RT_EXTERN - C and C++ mode
    // Missing 'extern' in linkage specification
    if (parser_test) {
        "C" void missing_extern_func();  // Error: expected 'extern'
    }
    
    // RT_STATIC_ASSERT - C11/C++11 and later
    // static_assert without the keyword
    if (parser_test) {
        #ifdef __cplusplus
        (1 == 1, "static assertion failed");  // Error: expected 'static_assert'
        #else
        _Static_assert  // Missing parentheses to trigger expected token
        #endif
    }
    
    #ifdef __cplusplus
    // C++-specific tokens
    
    // RT_DECLTYPE
    // decltype misspelled in trailing return type
    if (parser_test) {
        auto decltype_error() -> etype(parser_test);  // Error: expected 'decltype'
    }
    
    // RT_OPERATOR  
    // Operator overload without 'operator' keyword
    if (parser_test) {
        class MyClass {};
        int +(MyClass a, MyClass b) { return 0; }  // Error: expected 'operator'
    }
    
    // RT_CLASS
    // Class definition without 'class' keyword
    if (parser_test) {
        MyMissingClass {  // Error: expected 'class'
            int x;
        };
    }
    
    // RT_TEMPLATE
    // Template without 'template' keyword
    if (parser_test) {
        <typename T> void template_error() {}  // Error: expected 'template'
    }
    
    // RT_NAMESPACE
    // Namespace without 'namespace' keyword
    if (parser_test) {
        my_namespace {  // Error: expected 'namespace'
            int x;
        }
    }
    
    // RT_USING
    // Using directive without 'using' keyword
    if (parser_test) {
        namespace std;  // Error: expected 'using'
    }
    
    // RT_TRY
    // Try block without 'try' keyword
    if (parser_test) {
        {  // Error: expected 'try'
            throw 1;
        } catch (...) {}
    }
    #endif  // __cplusplus
    
    // RT_ASM - C and C++ mode
    // Inline assembly without 'asm' keyword
    if (parser_test) {
        #ifdef __GNUC__
        volatile ("nop");  // Error: expected 'asm'
        #endif
    }
    
    // Test with preprocessor macros to affect parser state
    #define BAD_TEMPLATE template
    #ifdef __cplusplus
    if (parser_test) {
        BAD_ <typename T> void f() {}  // Error after macro expansion
    }
    #endif
    
    // Test with attributes
    if (parser_test) {
        #ifdef __GNUC__
        void __attribute__((noreturn)) missing_extern_func2();  // Needs 'extern' for linkage
        #endif
    }
    
    // Nested in function-like context
    #ifdef __cplusplus
    if (parser_test) {
        namespace outer {
            class OuterClass {
                // Missing 'template' inside class
                <typename T> void member_func() {}  // Error: expected 'template'
            };
        }
    }
    #endif
    
    return 0;
}
