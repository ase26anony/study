#ifdef __cplusplus
#include <type_traits>
#endif

/* Use volatile to prevent dead code elimination */
volatile int parser_test_condition = 0;

/* Macro-based errors to test parser state during expansion */
#define BAD_CLASS class
#define BAD_EXTERN extern
#define BAD_ASM asm

int main() {
    /* Main function must be valid and compilable */
    volatile int test_case = 0;
    
    /* Test RT_EXTERN - C and C++ */
    if (parser_test_condition) {
        /* Missing 'extern' in linkage specification */
        "C" void missing_extern_func(void);  /* Should expect 'extern' */
    }
    
#ifdef __cplusplus
    /* C++ specific tests */
    
    /* Test RT_STATIC_ASSERT */
    if (test_case == 1) {
        /* Missing 'static_assert' keyword */
        (1 == 1, "static assertion failed");  /* Should expect 'static_assert' */
    }
    
    /* Test RT_DECLTYPE */
    if (test_case == 2) {
        /* Missing 'decltype' in trailing return type */
        auto missing_decltype_func() -> etype(1 + 2);  /* Should expect 'decltype' */
    }
    
    /* Test RT_OPERATOR */
    if (test_case == 3) {
        struct TestClass {};
        /* Missing 'operator' in overload declaration */
        int +(TestClass a, TestClass b) { return 0; }  /* Should expect 'operator' */
    }
    
    /* Test RT_CLASS */
    if (test_case == 4) {
        /* Missing 'class' keyword in definition */
        MissingClassKeyword {  /* Should expect 'class' */
            int x;
        };
    }
    
    /* Test RT_TEMPLATE */
    if (test_case == 5) {
        /* Missing 'template' keyword */
        <typename T> void missing_template_func() {}  /* Should expect 'template' */
    }
    
    /* Test RT_NAMESPACE */
    if (test_case == 6) {
        /* Missing 'namespace' keyword */
        MissingNamespace {  /* Should expect 'namespace' */
            int x;
        }
    }
    
    /* Test RT_USING */
    if (test_case == 7) {
        /* Missing 'using' keyword */
        namespace std;  /* Should expect 'using' */
    }
    
    /* Test RT_TRY */
    if (test_case == 8) {
        /* Missing 'try' keyword */
        {  /* Should expect 'try' */
            throw 1;
        } catch (...) {
        }
    }
#endif

    /* Test RT_ASM - C and C++ */
    if (test_case == 9) {
        /* Missing 'asm' keyword in inline assembly */
        volatile ("nop");  /* Should expect 'asm' */
    }
    
    /* Additional tests with attributes and macros */
    if (parser_test_condition) {
        /* Test with attribute - may affect parser state */
        __attribute__((unused)) MissingExtern "C" void attr_func(void);
    }
    
    /* Macro expansion errors */
    if (test_case == 10) {
        BAD_ MyType {};  /* BAD_ expands to nothing, expecting 'class' */
    }
    
    if (test_case == 11) {
        BAD_ "C" void macro_extern_func(void);  /* Expecting 'extern' */
    }
    
    /* Nested scope tests */
#ifdef __cplusplus
    namespace Outer {
        if (test_case == 12) {
            /* Missing 'namespace' inside another namespace */
            Inner {  /* Should expect 'namespace' */
                int y;
            }
        }
    }
    
    class Container {
        if (test_case == 13) {
            /* Missing 'class' inside a class */
            NestedClass {  /* Should expect 'class' */
                int z;
            };
        }
    };
#endif

    return 0;
}

/* Additional test cases in global scope */
#ifdef __cplusplus
/* Missing 'template' at global scope with attributes */
__attribute__((visibility("default")))
<typename T> void global_missing_template() {}  /* Should expect 'template' */

/* Missing 'static_assert' at global scope */
(1 == 1, "global static assertion");  /* Should expect 'static_assert' */
#endif

/* Missing 'extern' in global linkage spec */
"C" int global_var;  /* Should expect 'extern' */

/* Function with missing 'asm' */
void asm_test(void) {
    volatile ("nop");  /* Should expect 'asm' */
}
