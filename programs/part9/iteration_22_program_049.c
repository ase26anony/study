/* Targeted test for uncovered parser error recovery blocks in GCC */
#ifdef __cplusplus
#include <iostream>
#endif

volatile int parser_test_condition = 0;

int main() {
    /* Valid main function that will compile successfully */
    #ifdef __cplusplus
    std::cout << "Parser test" << std::endl;
    #endif
    
    /* Use volatile to control flow but ensure parser sees all code paths */
    volatile int test_case = 0;
    
    /* ===== RT_EXTERN ===== */
    /* C and C++: Missing 'extern' in linkage specification */
    if (test_case == 1) {
        /* ERROR: Expected 'extern' before string literal */
        "C" void missing_extern_func(void);
    }
    
    /* ===== RT_STATIC_ASSERT ===== */
    /* C11/C++11: Missing 'static_assert' keyword */
    #if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L || defined(__cplusplus) && __cplusplus >= 201103L
    if (test_case == 2) {
        /* ERROR: Expected 'static_assert' */
        (1 == 1, "static assertion failed");
    }
    #endif
    
    /* ===== C++ SPECIFIC TOKENS ===== */
    #ifdef __cplusplus
    
    /* ===== RT_DECLTYPE ===== */
    if (test_case == 3) {
        /* ERROR: Expected 'decltype' in trailing return type */
        auto missing_decltype_func() -> etype(x) { return x; }
    }
    
    /* ===== RT_OPERATOR ===== */
    if (test_case == 4) {
        class MyClass {
            int value;
        public:
            MyClass(int v) : value(v) {}
        };
        /* ERROR: Expected 'operator' in operator overload */
        int +(MyClass a, MyClass b) {
            return a.value + b.value;
        }
    }
    
    /* ===== RT_CLASS ===== */
    if (test_case == 5) {
        /* ERROR: Expected 'class' (or 'struct') */
        MissingClassKeyword {
            int x;
            void f();
        };
    }
    
    /* ===== RT_TEMPLATE ===== */
    if (test_case == 6) {
        /* ERROR: Expected 'template' before template parameters */
        <typename T>
        void missing_template_func(T t) {}
    }
    
    /* ===== RT_NAMESPACE ===== */
    if (test_case == 7) {
        /* ERROR: Expected 'namespace' */
        missing_namespace_keyword {
            int x;
        }
    }
    
    /* ===== RT_USING ===== */
    if (test_case == 8) {
        /* ERROR: Expected 'using' */
        namespace std;
    }
    
    /* ===== RT_TRY ===== */
    if (test_case == 9) {
        /* ERROR: Expected 'try' before block */
        {
            throw 42;
        }
        catch (...) {
            /* handler */
        }
    }
    
    #endif /* __cplusplus */
    
    /* ===== RT_ASM ===== */
    /* C and C++: Missing 'asm' keyword */
    if (test_case == 10) {
        /* ERROR: Expected 'asm' */
        volatile ("nop");
    }
    
    /* ===== TEST WITH MACRO EXPANSIONS ===== */
    /* Force parser to handle tokens from macro expansion */
    #define EXPECT_CLASS class
    #define EXPECT_TEMPLATE template
    
    if (test_case == 11) {
        /* ERROR: Macro expands to incomplete token sequence */
        EXPECT_ MyIncompleteType { int x; };
    }
    
    if (test_case == 12) {
        /* ERROR: Missing template keyword after macro */
        EXPECT_TEMPLATE_ <typename T> void f() {}
    }
    
    /* ===== TEST WITH ATTRIBUTES ===== */
    /* Place errors after attribute specifiers */
    if (test_case == 13) {
        /* ERROR: Attribute followed by invalid syntax expecting 'extern' */
        __attribute__((deprecated)) "C" void attr_missing_extern(void);
    }
    
    #ifdef __cplusplus
    if (test_case == 14) {
        /* ERROR: Attribute followed by class definition missing 'class' */
        [[nodiscard]] MissingClassAfterAttr {
            int value;
        };
    }
    #endif
    
    /* ===== NESTED CONTEXTS ===== */
    #ifdef __cplusplus
    namespace Outer {
        if (test_case == 15) {
            /* ERROR: In namespace scope, expecting 'namespace' */
            Inner {
                int x;
            };
        }
        
        class Container {
            if (test_case == 16) {
                /* ERROR: In class scope, expecting 'operator' */
                int +(int a, int b) { return a + b; }
            }
        };
    }
    #endif
    
    /* ===== VOLATILE-CONTROLLED DEAD CODE ===== */
    /* Ensure parser sees all branches even if not executed */
    volatile int dead_code = 0;
    
    if (dead_code) {
        /* ERROR: Multiple errors in dead code block */
        #ifdef __cplusplus
        <typename T> void dead1() {}
        class { int x; };
        #endif
        "C" void dead2();
    }
    
    return 0;
}
