#ifdef __cplusplus
#include <iostream>
#endif

/* Volatile control variable to ensure all code paths are parsed */
volatile int parse_control = 0;

int main() {
    /* Main function is valid and compilable */
    volatile int selector = 0;
    
    /* Each error block is isolated to prevent cascading failures */
    
    /* 1. RT_EXTERN: Missing 'extern' in linkage specification */
    if (selector == 1) {
        /* Error: missing 'extern' before linkage spec */
        "C" void missing_extern_func();  /* Should be: extern "C" */
    }
    
    /* 2. RT_STATIC_ASSERT: Missing 'static_assert' keyword */
    if (selector == 2) {
        /* Error: static_assert without keyword */
        (1 == 1, "Should be static_assert");  /* Should be: static_assert(1==1, "...") */
    }
    
#ifdef __cplusplus
    /* C++ specific tokens */
    
    /* 3. RT_DECLTYPE: Misspelled decltype in trailing return */
    if (selector == 3) {
        auto decltype_error() -> etype(selector);  /* Should be: decltype(selector) */
    }
    
    /* 4. RT_OPERATOR: Missing 'operator' in overload */
    if (selector == 4) {
        class MyClass {};
        int +(MyClass a, MyClass b) { return 0; }  /* Should be: operator+ */
    }
    
    /* 5. RT_CLASS: Missing 'class' keyword */
    if (selector == 5) {
        /* Error: missing 'class' before class name */
        MissingClassKeyword {  /* Should be: class MissingClassKeyword */
            int x;
        };
    }
    
    /* 6. RT_TEMPLATE: Missing 'template' keyword */
    if (selector == 6) {
        /* Error: missing 'template' */
        <typename T> void template_error() {}  /* Should be: template<typename T> */
    }
    
    /* 7. RT_NAMESPACE: Missing 'namespace' keyword */
    if (selector == 7) {
        /* Error: missing 'namespace' */
        my_namespace {  /* Should be: namespace my_namespace */
            int x;
        }
    }
    
    /* 8. RT_USING: Missing 'using' keyword */
    if (selector == 8) {
        /* Error: missing 'using' */
        namespace std;  /* Should be: using namespace std; */
    }
    
    /* 9. RT_TRY: Missing 'try' keyword */
    if (selector == 9) {
        /* Error: missing 'try' */
        {  /* Should be: try { */
            throw 1;
        } catch (...) {}
    }
#endif
    
    /* 10. RT_ASM: Missing 'asm' keyword (works in both C and C++) */
    if (selector == 10) {
        /* Error: missing 'asm' */
        volatile ("nop");  /* Should be: asm volatile ("nop") */
    }
    
    /* Additional tests with preprocessor and attributes */
    
    /* Test with macro expansion */
#define BAD_CLASS class
    if (selector == 11) {
        BAD_ MyType {};  /* Macro expands to incomplete token */
    }
    
    /* Test with __attribute__ */
    if (selector == 12) {
        void __attribute__((noreturn)) missing_extern_again() "C";  /* Missing extern after attribute */
    }
    
    /* Nested scope test for RT_CLASS */
    if (selector == 13) {
        namespace test_ns {
            MissingClassInsideNamespace {  /* Should be: class MissingClassInsideNamespace */
                int y;
            };
        }
    }
    
    /* Template with missing 'template' in class context */
#ifdef __cplusplus
    if (selector == 14) {
        class Outer {
            <typename T>  /* Missing 'template' inside class */
            class Inner {};
        };
    }
    
    /* Operator in template context */
    if (selector == 15) {
        template<typename T>
        T *(T a, T b) { return a; }  /* Missing 'operator' in template function */
    }
#endif
    
    /* Static assert in different contexts */
    if (selector == 16) {
        struct S {
            (sizeof(int) == 4, "int must be 4 bytes");  /* Missing static_assert in struct */
        };
    }
    
    /* Using directive with attributes */
#ifdef __cplusplus
    if (selector == 17) {
        __attribute__((visibility("default"))) 
        namespace boost;  /* Missing 'using' with attribute */
    }
#endif
    
    return 0;
}

/* Additional error cases in global scope for different parser states */

/* RT_EXTERN at file scope */
"C" int global_missing_extern;  /* Should be: extern "C" int ... */

#ifdef __cplusplus
/* RT_NAMESPACE at file scope with attributes */
__attribute__((visibility("hidden")))
hidden_ns {  /* Should be: namespace hidden_ns */
    int z;
}
#endif

/* RT_STATIC_ASSERT at file scope in C11/C++ */
#if __STDC_VERSION__ >= 201112L || __cplusplus >= 201103L
(1, "file scope static assert error");  /* Missing static_assert */
#endif

/* RT_ASM at file scope */
volatile int asm_global __asm__("missing_asm_var") = 0;  /* Valid, but test missing asm case */
if (parse_control) {  /* This if is invalid at file scope, triggers error recovery */
    volatile ("nop");  /* Missing asm */
}
