#ifdef __cplusplus
#include <iostream>
#endif

volatile int parser_mode = 0;

int main() {
    // Valid main function that compiles successfully
    #ifdef __cplusplus
    std::cout << "Parser test" << std::endl;
    #endif
    
    // Use volatile to control which error paths are parsed
    volatile int trigger = 0;
    
    // ===== RT_EXTERN =====
    // C and C++: Missing 'extern' in linkage specification
    if (trigger == 1) {
        // Missing 'extern' before "C"
        "C" void missing_extern_func();  // Error: expected 'extern'
    }
    
    // ===== RT_STATIC_ASSERT =====  
    // C11/C++: Missing 'static_assert' keyword
    if (trigger == 2) {
        // Missing 'static_assert' keyword
        (1 == 1, "static assert failed");  // Error: expected 'static_assert'
    }
    
#ifdef __cplusplus
    // C++-specific tokens
    
    // ===== RT_DECLTYPE =====
    // Missing 'decltype' in trailing return type
    if (trigger == 3) {
        auto missing_decltype_func() -> etype(5 + 3);  // Error: expected 'decltype'
    }
    
    // ===== RT_OPERATOR =====
    // Missing 'operator' in operator overload
    if (trigger == 4) {
        class MyClass4 {};
        // Missing 'operator' keyword
        MyClass4 +(MyClass4 a, MyClass4 b) { return a; }  // Error: expected 'operator'
    }
    
    // ===== RT_CLASS =====
    // Missing 'class' in class definition
    if (trigger == 5) {
        // Missing 'class' keyword
        MissingClassKeyword5 {  // Error: expected 'class'
            int x;
        };
    }
    
    // ===== RT_TEMPLATE =====
    // Missing 'template' keyword
    if (trigger == 6) {
        // Missing 'template' keyword
        <typename T> void missing_template_func() {}  // Error: expected 'template'
    }
    
    // ===== RT_NAMESPACE =====
    // Missing 'namespace' keyword
    if (trigger == 7) {
        // Missing 'namespace' keyword  
        MissingNamespace7 {  // Error: expected 'namespace'
            int x;
        }
    }
    
    // ===== RT_USING =====
    // Missing 'using' keyword
    if (trigger == 8) {
        // Missing 'using' keyword
        namespace std;  // Error: expected 'using'
    }
    
    // ===== RT_TRY =====
    // Missing 'try' keyword
    if (trigger == 9) {
        // Missing 'try' keyword
        {  // Error: expected 'try'
            throw 1;
        } catch (...) {}
    }
#endif
    
    // ===== RT_ASM =====
    // Missing 'asm' keyword (works in both C and C++)
    if (trigger == 10) {
        // Missing 'asm' keyword
        volatile ("nop");  // Error: expected 'asm'
    }
    
    // Additional tests with preprocessor and attributes
    
    // ===== RT_EXTERN with attribute =====
    if (trigger == 11) {
        __attribute__((weak)) "C" void attr_extern_func();  // Error: expected 'extern'
    }
    
#ifdef __cplusplus
    // ===== RT_CLASS with macro =====
    if (trigger == 12) {
        #define CLASS class
        // Macro expansion issue
        CLASS_ MyClass12 {};  // Error: expected 'class' (after macro expansion)
    }
    
    // ===== RT_TEMPLATE nested in class =====
    if (trigger == 13) {
        class Outer13 {
            // Missing 'template' inside class
            <typename T> void inner_func() {}  // Error: expected 'template'
        };
    }
    
    // ===== RT_OPERATOR in namespace =====
    if (trigger == 14) {
        namespace Test14 {
            class MyClass14 {};
            // Missing 'operator' in namespace scope
            MyClass14 *(MyClass14 a, MyClass14 b) { return a; }  // Error: expected 'operator'
        }
    }
    
    // ===== RT_USING with template =====
    if (trigger == 15) {
        template<typename T>
        class Container15 {};
        
        // Missing 'using' for template alias
        template<typename U> Alias15 = Container15<U>;  // Error: expected 'using'
    }
#endif
    
    // ===== RT_STATIC_ASSERT in C mode =====
    // Test with __extension__ to affect parser state
    if (trigger == 16) {
        __extension__ (1, "test");  // Error: expected 'static_assert' in proper context
    }
    
    return 0;
}

// Additional isolated error contexts in different scopes

#ifdef __cplusplus
// Global scope C++ errors
namespace GlobalTest {
    // RT_NAMESPACE at global scope
    MissingGlobalNamespace {  // Error: expected 'namespace'
        int global_var;
    }
}

// RT_DECLTYPE in function template
template<typename T>
auto template_func(T x) -> etype(x) {  // Error: expected 'decltype'
    return x;
}

// RT_TRY in function
void test_try_function() {
    {  // Error: expected 'try'
        throw "error";
    } catch (const char* e) {}
}
#endif

// RT_EXTERN at file scope (C compatible)
"C" void file_scope_extern_missing();  // Error: expected 'extern'

// RT_ASM in function
void test_asm_function() {
    volatile ("nop");  // Error: expected 'asm'
}
