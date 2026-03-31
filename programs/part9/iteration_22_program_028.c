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
    #if __STDC_VERSION__ >= 201112L || __cplusplus >= 201103L
    if (parser_test) {
        static_assert (1, "fail");  // Valid
        (1, "fail");  // Error: expected 'static_assert'
    }
    #endif
    
    #ifdef __cplusplus
    // RT_DECLTYPE - C++11 onwards
    if (parser_test) {
        auto missing_decltype_func() -> etype(x);  // Error: expected 'decltype'
    }
    
    // RT_OPERATOR - C++ only
    if (parser_test) {
        struct MyClass {};
        int +(MyClass a, MyClass b);  // Error: expected 'operator'
    }
    
    // RT_CLASS - C++ only
    if (parser_test) {
        MyClass { public: int x; };  // Error: expected 'class'
    }
    
    // RT_TEMPLATE - C++ only
    if (parser_test) {
        <typename T> void missing_template_func() {}  // Error: expected 'template'
    }
    
    // RT_NAMESPACE - C++ only
    if (parser_test) {
        my_ns { int x; };  // Error: expected 'namespace'
    }
    
    // RT_USING - C++ only
    if (parser_test) {
        namespace std;  // Error: expected 'using'
    }
    
    // RT_TRY - C++ only
    if (parser_test) {
        { throw 1; } catch (...) {}  // Error: expected 'try'
    }
    #endif
    
    // RT_ASM - C and C++ (GCC extension)
    if (parser_test) {
        __asm__ volatile ("nop");  // Valid
        volatile ("nop");  // Error: expected 'asm'
    }
    
    // Test with preprocessor macros
    #define BAD_CLASS class
    #define BAD_TEMPLATE template
    
    #ifdef __cplusplus
    if (parser_test) {
        BAD_ MyType {};  // Error after macro expansion
        BAD_TEMPLATE_ <typename T> void f() {}  // Error after macro
    }
    #endif
    
    // Test with attributes (affects parser state)
    if (parser_test) {
        #ifdef __cplusplus
        [[noreturn]] void missing_extern_func2();  // Needs 'extern' for C linkage
        #else
        __attribute__((noreturn)) void missing_extern_func2();  // Same in C
        #endif
    }
    
    // Nested contexts
    #ifdef __cplusplus
    namespace outer {
        if (parser_test) {
            inner { int x; };  // Error: expected 'namespace' inside namespace
        }
    }
    
    class Container {
        if (parser_test) {
            public:  // Syntax error in class scope
            <typename T> void method();  // Error: expected 'template'
        }
    };
    #endif
    
    return 0;
}

// Additional test cases in different scopes
#ifdef __cplusplus
// Global scope C++ errors
template<typename T>
void template_func() {
    if (flag) {
        operator+(T, T);  // Error: missing return type before 'operator'
    }
}

// Try block without try
void test_try() {
    { throw 42; } catch(int x) { }  // Error: expected 'try'
}
#endif

// C-specific test (compile with -xc)
#ifndef __cplusplus
// Static assert error in C
_Static_assert(1, "ok");
(1, "fail");  // Error: expected '_Static_assert' or 'static_assert'

// Extern error in C
"C" int c_func(void);  // Error: expected 'extern'
#endif
