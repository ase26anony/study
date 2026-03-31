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
    volatile int parser_test = 0;
    
    // RT_EXTERN - C and C++ mode
    // Missing 'extern' in linkage specification
    if (parser_test) {
        "C" void missing_extern_func();  // Error: expected 'extern'
    }
    
    // RT_STATIC_ASSERT - C11 and C++11 onwards
    // static_assert without keyword
    if (parser_test) {
        #ifdef __cplusplus
        (1 == 1, "static assertion");  // Error: expected 'static_assert'
        #else
        _Static_assert(1 == 1, "C static assert");  // Valid C11
        (1 == 1, "C assertion");  // Error in C mode too
        #endif
    }
    
#ifdef __cplusplus
    // C++ specific tokens
    
    // RT_DECLTYPE - missing in trailing return type
    if (parser_test) {
        auto missing_decltype_func() -> etype(x);  // Error: expected 'decltype'
    }
    
    // RT_OPERATOR - operator overload without keyword
    if (parser_test) {
        class MyClass {};
        int +(MyClass a, MyClass b) { return 0; }  // Error: expected 'operator'
    }
    
    // RT_CLASS - class definition without keyword
    if (parser_test) {
        MissingClassKeyword {  // Error: expected 'class'
            int x;
        };
    }
    
    // RT_TEMPLATE - template without keyword
    if (parser_test) {
        <typename T> void template_missing() {}  // Error: expected 'template'
    }
    
    // RT_NAMESPACE - namespace without keyword
    if (parser_test) {
        MissingNamespace {  // Error: expected 'namespace'
            int x;
        }
    }
    
    // RT_USING - using directive without keyword
    if (parser_test) {
        namespace std;  // Error: expected 'using'
    }
    
    // RT_TRY - try block without keyword
    if (parser_test) {
        {  // Error: expected 'try'
            throw 1;
        } catch (...) {}
    }
#endif

    // RT_ASM - inline assembly without asm keyword
    // Test in both C and C++ modes
    if (parser_test) {
        #ifdef __cplusplus
        volatile ("nop");  // Error: expected 'asm'
        #else
        __asm__ volatile ("nop");  // Valid GNU C
        volatile ("nop");  // Error in C mode
        #endif
    }
    
    // Additional tests with preprocessor and attributes
    
    // Macro expansion causing missing token
    #define BAD_CLASS class
    if (parser_test) {
        BAD_ MyType {};  // Error after macro expansion
    }
    
    // Attribute before error
    if (parser_test) {
        #ifdef __cplusplus
        [[noreturn]] void missing_extern_func2();  // Valid
        [[noreturn]] "C" void missing_extern_func3();  // Error: expected 'extern'
        #else
        __attribute__((noreturn)) "C" void missing_extern_func4();  // Error
        #endif
    }
    
    // Nested in function for scope testing
    if (parser_test) {
        void nested_function() {
            #ifdef __cplusplus
            // Try block error in nested scope
            { throw 2; } catch (...) {}  // Error: expected 'try'
            #endif
        }
    }
    
    return 0;
}
