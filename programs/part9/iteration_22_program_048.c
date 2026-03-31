/* Test program to trigger uncovered error recovery blocks in gcc/parser.cc */
/* Compile with: g++ -xc++ -fsyntax-only -O0 -fdiagnostics-parseable-fixits test.cc */
/* Or for C: gcc -xc -fsyntax-only -O1 -fno-show-column test.cc */

#ifdef __cplusplus
#include <iostream>
#endif

/* Use volatile to prevent dead code elimination */
static volatile int parser_test_condition = 0;

/* Macro-based error to test parser state during expansion */
#define BAD_CLASS class
#define BAD_TEMPLATE template
#define BAD_NAMESPACE namespace

int main() {
    /* Main function is valid and compilable */
    
    /* Use volatile condition to control which error blocks are parsed */
    volatile int test_case = 0;
    
    /* Each error block is isolated to prevent cascade failures */
    
    /* 1. RT_EXTERN: Missing 'extern' in linkage specification */
    if (test_case == 1) {
        /* Missing 'extern' before "C" */
        "C" void missing_extern_func(void);  /* Error: expected 'extern' */
    }
    
    /* 2. RT_STATIC_ASSERT: Missing 'static_assert' keyword */
    if (test_case == 2) {
        /* Missing 'static_assert' keyword */
        (1 == 1, "static assertion failed");  /* Error: expected 'static_assert' */
    }
    
#ifdef __cplusplus
    /* C++ specific tokens */
    
    /* 3. RT_DECLTYPE: Misspelled decltype in trailing return type */
    if (test_case == 3) {
        auto decltype_error() -> etype(parser_test_condition);  /* Error: expected 'decltype' */
    }
    
    /* 4. RT_OPERATOR: Missing 'operator' in overload definition */
    if (test_case == 4) {
        struct MyClass {
            int x;
        };
        /* Missing 'operator' keyword */
        int +(MyClass a, MyClass b) { return a.x + b.x; }  /* Error: expected 'operator' */
    }
    
    /* 5. RT_CLASS: Missing 'class' in class definition */
    if (test_case == 5) {
        /* Missing 'class' keyword */
        MissingClassKeyword {  /* Error: expected 'class' */
            int x;
        };
    }
    
    /* 6. RT_TEMPLATE: Missing 'template' keyword */
    if (test_case == 6) {
        /* Missing 'template' keyword */
        <typename T>  /* Error: expected 'template' */
        void template_error() {}
    }
    
    /* 7. RT_NAMESPACE: Missing 'namespace' keyword */
    if (test_case == 7) {
        /* Missing 'namespace' keyword */
        my_namespace {  /* Error: expected 'namespace' */
            int x;
        }
    }
    
    /* 8. RT_USING: Missing 'using' in using directive */
    if (test_case == 8) {
        /* Missing 'using' keyword */
        namespace std;  /* Error: expected 'using' */
    }
    
    /* 9. RT_TRY: Missing 'try' in try-catch block */
    if (test_case == 9) {
        /* Missing 'try' keyword */
        {  /* Error: expected 'try' */
            throw 1;
        } catch (...) {
        }
    }
    
    /* Test with macro expansion */
    if (test_case == 10) {
        /* Using macro that expands to partial keyword */
        BAD_ MyMacroClass {};  /* Error: expected 'class' after macro expansion */
    }
    
    /* Test with attributes */
    if (test_case == 11) {
        __attribute__((deprecated))
        <typename T> void attr_error() {}  /* Error: expected 'template' after attribute */
    }
    
    /* Nested in namespace */
    namespace test_ns {
        if (test_case == 12) {
            /* Missing 'class' inside namespace */
            NestedClass {  /* Error: expected 'class' */
                int y;
            };
        }
    }
#endif /* __cplusplus */

    /* 10. RT_ASM: Missing 'asm' in inline assembly (works in C and C++) */
    if (test_case == 13) {
        /* Missing 'asm' keyword */
        volatile ("nop");  /* Error: expected 'asm' */
    }
    
    /* C-specific variant for RT_EXTERN */
    if (test_case == 14) {
        /* In C mode, test extern in different context */
        "C" {  /* Error: expected 'extern' */
            int c_func(void);
        }
    }
    
    /* C-specific variant for RT_STATIC_ASSERT (C11) */
    if (test_case == 15) {
        /* _Static_assert in C, but missing keyword */
        (1 == 1, "C static assertion");  /* Error: expected '_Static_assert' or 'static_assert' */
    }
    
    /* Test in function scope */
    if (test_case == 16) {
        void inner_function(void) {
            /* Missing 'asm' inside function */
            volatile ("nop");  /* Error: expected 'asm' */
        }
    }
    
    return 0;
}

/* Additional test cases outside main to ensure parser encounters them */
#ifdef __cplusplus
/* Global scope error for RT_CLASS */
BAD_ GlobalClass {};  /* Error: expected 'class' */

/* Template with missing 'template' at global scope */
<typename U>  /* Error: expected 'template' */
void global_template_error() {}

namespace {
    /* Anonymous namespace error for RT_USING */
    namespace inner {}
    inner;  /* Error: expected 'using' */
}
#endif

/* C mode tests */
#ifndef __cplusplus
/* C linkage specification error */
"C" void c_linkage_error(void);  /* Error: expected 'extern' */

/* _Static_assert error in C */
_Static_assert (1 == 2, "C assert");  /* Valid, no error */
(1 == 2, "Missing _Static_assert");  /* Error: expected '_Static_assert' */
#endif
