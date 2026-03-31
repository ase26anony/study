#ifdef __cplusplus
#include <iostream>
#endif

volatile int parser_mode = 0;

int main() {
    // Valid main function that will compile
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
        auto missing_decltype_func() -> (x);  // Error: expected 'decltype'
    }
    
    // ===== RT_OPERATOR =====
    // Missing 'operator' in overload definition
    if (trigger == 4) {
        class MyClass4 {};
        int +(MyClass4 a, MyClass4 b) { return 0; }  // Error: expected 'operator'
    }
    
    // ===== RT_CLASS =====
    // Missing 'class' keyword in class definition
    if (trigger == 5) {
        MissingClassKeyword {  // Error: expected 'class'
            int x;
        };
    }
    
    // ===== RT_TEMPLATE =====
    // Missing 'template' keyword
    if (trigger == 6) {
        <typename T> void missing_template_func() {}  // Error: expected 'template'
    }
    
    // ===== RT_NAMESPACE =====
    // Missing 'namespace' keyword
    if (trigger == 7) {
        MissingNamespaceKeyword {  // Error: expected 'namespace'
            int y;
        }
    }
    
    // ===== RT_USING =====
    // Missing 'using' keyword
    if (trigger == 8) {
        namespace std;  // Error: expected 'using'
    }
    
    // ===== RT_TRY =====
    // Missing 'try' keyword
    if (trigger == 9) {
        {  // Error: expected 'try'
            throw 1;
        } catch (...) {}
    }
#endif
    
    // ===== RT_ASM =====
    // Missing 'asm' keyword in inline assembly (C and C++)
    if (trigger == 10) {
        volatile ("nop");  // Error: expected 'asm'
    }
    
    // Additional tests with preprocessor and attributes
    
    // Test with macros
    #define BAD_TEMPLATE template
    if (trigger == 11) {
        BAD_ <int> void macro_func() {}  // Error after macro expansion
    }
    
    // Test with __attribute__
    if (trigger == 12) {
        void __attribute__((noreturn)) missing_extern_attr() "C";  // Error: expected 'extern'
    }
    
    // Nested scope tests
    if (trigger == 13) {
        namespace Outer {
            // Missing 'class' inside namespace
            InnerClass {  // Error: expected 'class'
                int z;
            };
        }
    }
    
    // Multiple errors in different scopes (only one active at a time)
    if (trigger == 14) {
        // Function scope error for 'operator'
        class MyClass14 {
            int operator()(int x) { return x; }  // Valid
            int +(int x) { return x; }  // Error: expected 'operator'
        };
    }
    
    // Template context error
    if (trigger == 15) {
        template<typename T>
        class TemplateClass {
            // Missing 'template' in dependent context
            <typename U> void nested_template();  // Error: expected 'template'
        };
    }
    
    return 0;
}

// Additional top-level errors in dead code
#ifdef __cplusplus
// Missing 'namespace' at file scope (guarded by #if 0)
#if 0
MissingTopLevelNamespace {
    int global_var;
}
#endif

// Missing 'class' with inheritance
#if 0
DerivedClass : public BaseClass {  // Error: expected 'class'
    int member;
};
#endif
#endif

// C-specific variants
#ifndef __cplusplus
// C mode tests
#if 0
// Missing 'extern' in C
"C" int c_missing_extern(void);  // Error: expected 'extern'

// Missing 'asm' in C
void c_function(void) {
    volatile ("nop");  // Error: expected 'asm'
}

// _Static_assert in C11
_Static_assert(1 == 1, "C static assert");  // Valid
(1 == 1, "missing static assert");  // Error
#endif
#endif
