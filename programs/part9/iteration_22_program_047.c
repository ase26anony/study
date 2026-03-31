#ifdef __cplusplus
#include <iostream>
#endif

/* Volatile control variable to prevent dead code elimination */
volatile int parser_test_mode = 0;

/* Macro-based error to test parser state machine */
#define BAD_CLASS class
#define BAD_TEMPLATE template

int main() {
    /* Main function is valid and compilable */
    volatile int test_case = 0;
    
    /* RT_EXTERN - C and C++ */
    if (test_case == 1) {
        /* Missing 'extern' in linkage specification */
        "C" void extern_func();  /* Error: expected 'extern' */
    }
    
    /* RT_STATIC_ASSERT - C11 and C++ */
    if (test_case == 2) {
        /* Missing 'static_assert' keyword */
        (1 == 1, "static assertion failed");  /* Error: expected 'static_assert' */
    }
    
#ifdef __cplusplus
    /* C++-specific tokens */
    
    /* RT_DECLTYPE */
    if (test_case == 3) {
        /* Missing 'decltype' in trailing return type */
        auto decltype_func() -> etype(5 + 3);  /* Error: expected 'decltype' */
    }
    
    /* RT_OPERATOR */
    if (test_case == 4) {
        struct MyClass {
            int value;
        };
        /* Missing 'operator' in overload definition */
        int +(MyClass a, MyClass b) {  /* Error: expected 'operator' */
            return a.value + b.value;
        }
    }
    
    /* RT_CLASS */
    if (test_case == 5) {
        /* Missing 'class' keyword in definition */
        MyClassType {  /* Error: expected 'class' */
            int x;
        };
    }
    
    /* RT_TEMPLATE */
    if (test_case == 6) {
        /* Missing 'template' keyword */
        <typename T>  /* Error: expected 'template' */
        void template_func() {}
    }
    
    /* RT_NAMESPACE */
    if (test_case == 7) {
        /* Missing 'namespace' keyword */
        my_namespace {  /* Error: expected 'namespace' */
            int y;
        }
    }
    
    /* RT_USING */
    if (test_case == 8) {
        /* Missing 'using' keyword */
        namespace std;  /* Error: expected 'using' */
    }
    
    /* RT_TRY */
    if (test_case == 9) {
        /* Missing 'try' keyword */
        {  /* Error: expected 'try' */
            throw 42;
        } catch (...) {
        }
    }
#endif
    
    /* RT_ASM - C and C++ */
    if (test_case == 10) {
        /* Missing 'asm' keyword for inline assembly */
        __volatile__ ("nop");  /* Error: expected 'asm' */
    }
    
    /* Additional tests with preprocessor interactions */
#if 0
    /* RT_CLASS via macro expansion */
    BAD_ MyMacroClass {  /* After macro expansion: expected 'class' */
        int z;
    };
    
    /* RT_TEMPLATE via macro with attributes */
    __attribute__((deprecated))
    BAD_ <int> void attr_func() {}  /* Error: expected 'template' */
#endif
    
    /* Nested scope tests */
    if (test_case == 11) {
        void nested_function() {
            /* RT_EXTERN in nested function */
            "C++" void nested_extern();  /* Error: expected 'extern' */
        }
    }
    
#ifdef __cplusplus
    if (test_case == 12) {
        namespace Outer {
            /* RT_CLASS inside namespace */
            InnerClass {  /* Error: expected 'class' */
                double d;
            };
        }
    }
    
    if (test_case == 13) {
        class Container {
            /* RT_OPERATOR inside class */
            int +(const Container& other);  /* Error: expected 'operator' */
        };
    }
#endif
    
    /* Test with different parser states */
    if (test_case == 14) {
        /* RT_STATIC_ASSERT after attribute */
        __attribute__((unused))
        (1, "test");  /* Error: expected 'static_assert' */
    }
    
    return 0;
}

/* Additional top-level error cases */
#ifdef __cplusplus
/* RT_TEMPLATE at file scope */
<typename U>  /* Error: expected 'template' */
struct FileScopeTemplate {
    U data;
};
#endif

/* RT_EXTERN at file scope with wrong syntax */
"C"  /* Error: expected 'extern' */
int global_var = 0;

/* RT_ASM at file scope */
__volatile__ ("mov %eax, %ebx");  /* Error: expected 'asm' */
