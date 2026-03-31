/* Test program to trigger specific parser error recovery paths */
#ifdef __cplusplus
#include <iostream>
#endif

/* Volatile control variables to ensure code is parsed but not necessarily executed */
static volatile int parser_test_cond = 0;
static volatile int test_case = 0;

/* Macro-based error to test parser state during macro expansion */
#define BAD_CLASS class
#define BAD_EXTERN extern
#define BAD_ASM asm

int main() {
    /* Main function is valid and compilable */
    volatile int v = 0;
    
    /* Test case 0: RT_EXTERN - C mode */
    if (v == 999) { /* Always false, but parsed */
        /* Missing 'extern' in linkage specification */
        "C" void missing_extern_func(void);  /* Should trigger: expected 'extern' */
    }
    
    /* Test case 1: RT_STATIC_ASSERT - C11/C++11 mode */
    if (v == 998) {
        /* Missing 'static_assert' keyword */
        (1 == 1, "static assertion failed");  /* Should trigger: expected 'static_assert' */
    }
    
#ifdef __cplusplus
    /* C++ specific tests */
    
    /* Test case 2: RT_DECLTYPE - C++11 */
    if (v == 997) {
        auto missing_decltype_func() -> etype(x) { return 0; }  /* Should trigger: expected 'decltype' */
    }
    
    /* Test case 3: RT_OPERATOR */
    if (v == 996) {
        struct MyClass {
            int value;
        };
        /* Missing 'operator' in overload */
        int +(MyClass a, MyClass b) { return a.value + b.value; }  /* Should trigger: expected 'operator' */
    }
    
    /* Test case 4: RT_CLASS */
    if (v == 995) {
        /* Missing 'class' keyword */
        MissingClassKeyword {  /* Should trigger: expected 'class' */
            int x;
        };
    }
    
    /* Test case 5: RT_TEMPLATE */
    if (v == 994) {
        /* Missing 'template' keyword */
        <typename T> void missing_template_func() {}  /* Should trigger: expected 'template' */
    }
    
    /* Test case 6: RT_NAMESPACE */
    if (v == 993) {
        /* Missing 'namespace' keyword */
        missing_namespace {  /* Should trigger: expected 'namespace' */
            int y;
        }
    }
    
    /* Test case 7: RT_USING */
    if (v == 992) {
        /* Missing 'using' keyword */
        namespace std;  /* Should trigger: expected 'using' */
    }
    
    /* Test case 8: RT_TRY */
    if (v == 991) {
        /* Missing 'try' keyword */
        {  /* Should trigger: expected 'try' */
            throw 1;
        } catch (...) {
        }
    }
#endif
    
    /* Test case 9: RT_ASM - Both C and C++ */
    if (v == 990) {
        /* Missing 'asm' keyword */
        __volatile__ ("nop");  /* Should trigger: expected 'asm' */
    }
    
    /* Additional tests with preprocessor interactions */
#if 0  /* Not compiled, but shows pattern */
    /* Macro expansion causing missing token */
    BAD_ MyType {};  /* BAD_ expands to nothing, expecting 'class' */
    
    /* Attribute interaction */
    __attribute__((deprecated)) missing_extern "C" void f();  /* Should trigger after attribute */
#endif
    
    /* Nested scope tests */
    if (v == 989) {
        void nested_function() {
            /* Missing 'static_assert' inside function */
            (sizeof(int) == 4, "int must be 4 bytes");  /* Should trigger: expected 'static_assert' */
        }
    }
    
#ifdef __cplusplus
    if (v == 988) {
        namespace Outer {
            /* Missing 'namespace' inside another namespace */
            Inner {  /* Should trigger: expected 'namespace' */
                int z;
            }
        }
    }
    
    /* Class scope test */
    if (v == 987) {
        class Container {
            /* Missing 'template' in member template */
            <typename U> void member_func() {}  /* Should trigger: expected 'template' */
        };
    }
#endif
    
    /* Test with volatile-controlled switch to ensure all paths are parsed */
    switch (test_case) {
        case 0:
            /* RT_EXTERN variant in C context */
            if (parser_test_cond) {
                extern missing_linkage "C" int global_var;  /* Wrong placement, expecting something */
            }
            break;
            
#ifdef __cplusplus
        case 1:
            /* RT_OPERATOR in expression context */
            if (parser_test_cond) {
                struct Num {
                    Num operator+(const Num&) const;
                };
                Num a, b;
                Num c = a + b;  /* Valid */
                Num d = a  b;   /* Missing operator, but different error */
            }
            break;
#endif
            
        default:
            break;
    }
    
    return 0;
}

/* Standalone errors in different contexts */
#ifdef __cplusplus
/* At namespace scope - missing template */
<typename T>  /* Should trigger: expected 'template' */
void standalone_func() {}

/* After attribute - missing extern */
__attribute__((weak)) "C"  /* Should trigger: expected 'extern' */
void attributed_func() {}
#endif

/* C-specific at file scope */
static missing_static_assert (1, "C static assert");  /* Should trigger: expected 'static_assert' */

/* Incomplete declarations to trigger expectations */
#ifdef __cplusplus
class;  /* Valid forward declaration */
;       /* Invalid, but different error */

namespace;  /* Invalid: expected namespace name */

using;  /* Invalid: expected namespace name or using-declaration */
#endif
