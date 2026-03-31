/* Test program to trigger specific parser error recovery paths */
#ifdef __cplusplus
#include <iostream>
#endif

/* Volatile control variable to ensure parsing of all branches */
volatile int parser_test_condition = 0;

/* Macro-based error to test parser state during macro expansion */
#define BAD_CLASS class
#define BAD_EXTERN extern
#define BAD_TEMPLATE template

int main() {
    /* Valid main function that conditionally includes erroneous code */
    
    /* RT_EXTERN - missing 'extern' in linkage specification */
    if (parser_test_condition == 1) {
        /* C and C++ compatible */
        "C" void missing_extern_func(); /* Error: expected 'extern' */
    }
    
#ifdef __cplusplus
    /* C++ specific errors */
    
    /* RT_STATIC_ASSERT - missing 'static_assert' keyword */
    if (parser_test_condition == 2) {
        (1 == 1, "static assertion failed"); /* Error: expected 'static_assert' */
    }
    
    /* RT_DECLTYPE - misspelled decltype in trailing return */
    if (parser_test_condition == 3) {
        auto decltype_error() -> etype(parser_test_condition); /* Error: expected 'decltype' */
    }
    
    /* RT_OPERATOR - missing 'operator' in overload definition */
    if (parser_test_condition == 4) {
        struct MyClass {
            int x;
        };
        int +(MyClass a, MyClass b) { return a.x + b.x; } /* Error: expected 'operator' */
    }
    
    /* RT_CLASS - missing 'class' keyword in definition */
    if (parser_test_condition == 5) {
        /* Using macro to obscure the error */
        BAD_ MissingClass { /* Error: expected 'class' */
            int member;
        };
    }
    
    /* RT_TEMPLATE - missing 'template' keyword */
    if (parser_test_condition == 6) {
        /* Direct error */
        <typename T> void template_missing() {} /* Error: expected 'template' */
        
        /* Macro-based variant */
        BAD_ <int> void macro_template() {} /* Error: expected 'template' */
    }
    
    /* RT_NAMESPACE - missing 'namespace' keyword */
    if (parser_test_condition == 7) {
        /* Inside a function to test scoping */
        {
            missing_ns { /* Error: expected 'namespace' */
                int x;
            }
        }
    }
    
    /* RT_USING - missing 'using' keyword */
    if (parser_test_condition == 8) {
        /* After attribute to test parser state */
        __attribute__((unused))
        namespace std; /* Error: expected 'using' */
    }
    
    /* RT_TRY - missing 'try' keyword */
    if (parser_test_condition == 9) {
        {
            throw 42;
        } catch (...) { /* Error: expected 'try' */
            // Handle
        }
    }
#endif
    
    /* RT_ASM - missing 'asm' keyword (C and C++) */
    if (parser_test_condition == 10) {
        /* Inline assembly without 'asm' */
        __volatile__ ("nop"); /* Error: expected 'asm' */
        
        /* With attribute */
        __attribute__((naked))
        ("mov %eax, %ebx"); /* Error: expected 'asm' */
    }
    
    /* Additional C-specific tests */
#ifndef __cplusplus
    /* RT_EXTERN in C mode with different context */
    if (parser_test_condition == 11) {
        /* Linkage specification in C */
        "C" { /* Error: expected 'extern' */
            int c_func(void);
        }
    }
    
    /* RT_STATIC_ASSERT in C11 */
    if (parser_test_condition == 12) {
        /* _Static_assert is C11, but parser might expect static_assert */
        _Static_assert(1, "C static assert");
        /* Also test missing version */
        (1, "missing static_assert"); /* Error: expected 'static_assert' */
    }
#endif
    
    /* Nested scope test for RT_CLASS */
#ifdef __cplusplus
    if (parser_test_condition == 13) {
        namespace outer {
            /* Missing class inside namespace */
            InnerClass { /* Error: expected 'class' */
                void method();
            };
        }
    }
    
    /* Template with missing 'template' in class context */
    if (parser_test_condition == 14) {
        struct Container {
            /* Missing template in member template */
            <typename U> U convert(); /* Error: expected 'template' */
        };
    }
    
    /* Missing 'operator' in more complex expression */
    if (parser_test_condition == 15) {
        class Complex {
        public:
            /* Conversion operator without 'operator' */
            int() const { return 42; } /* Error: expected 'operator' */
        };
    }
    
    /* Missing 'decltype' in SFINAE context */
    if (parser_test_condition == 16) {
        template<typename T>
        auto sfinae_test(T t) -> etype(t.begin()) { /* Error: expected 'decltype' */
            return t.begin();
        }
    }
#endif
    
    /* Multiple errors in different conditional branches */
    /* Each is guarded so parser sees them independently */
#if 0
    /* RT_EXTERN - This block is never compiled but will be parsed */
    "C++" void never_called();
#endif
    
#if 0
    /* RT_TEMPLATE */
    <class T> void unused() {}
#endif
    
    /* Valid return to ensure main compiles */
    return 0;
}

/* Additional error cases at file scope */

/* RT_EXTERN at file scope */
#ifdef __cplusplus
extern "C" {
#endif

/* Missing extern in linkage at file scope */
"C" int file_scope_func(void); /* Error: expected 'extern' */

#ifdef __cplusplus
}
#endif

/* RT_STATIC_ASSERT at file scope (C++ only) */
#ifdef __cplusplus
static_assert(1 == 1, "Always true");
/* Missing version */
// (1 == 1, "Missing static_assert"); /* Would cause error if uncommented */
#endif

/* RT_ASM at file scope in GNU C */
__asm__ volatile ("nop");
/* Missing asm variant */
// volatile ("nop"); /* Would cause error if uncommented */

/* Complex case with attributes and missing tokens */
#ifdef __cplusplus
class __attribute__((aligned(16))) MissingKeyword { /* Error: expected 'class' */
    template<typename T>
    __attribute__((always_inline))
    <T> void problematic(); /* Error: expected 'template' */
};
#endif

/* Final test: conditional compilation of all errors */
#define TEST_ERROR_CASE(n) \
    if (parser_test_condition == (100 + n)) { \
        /* Each case here would contain one error */ \
    }

/* Macro that expands to missing token */
#define EXPECT_CLASS class
#define EXPECT_TEMPLATE template

/* Use the macros to create errors */
#ifdef __cplusplus
EXPECT_ MissingFromMacro { int x; }; /* Error: expected 'class' */
EXPECT_ <int> void from_macro() {} /* Error: expected 'template' */
#endif
