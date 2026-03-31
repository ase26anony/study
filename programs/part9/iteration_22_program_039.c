/* Test program for GCC parser error recovery diagnostics */
#ifdef __cplusplus
#include <iostream>
#endif

/* Volatile control variable to ensure all code paths are parsed */
volatile int parser_test_condition = 0;

/* Macro-based error to test parser state during macro expansion */
#define BAD_CLASS class
#define BAD_EXTERN extern
#define BAD_ASM asm

int main() {
    /* Main function is valid and compilable */
    
    /* Use volatile to control which error blocks are parsed */
    volatile int test_case = 0;
    
    /* Block 1: RT_EXTERN error - C and C++ */
    if (parser_test_condition == 1) {
        /* Missing 'extern' in linkage specification */
        "C" void missing_extern_func();  /* Error: expected 'extern' */
    }
    
    /* Block 2: RT_STATIC_ASSERT error - C11 and C++ */
    if (parser_test_condition == 2) {
        /* Missing 'static_assert' keyword */
        (1 == 1, "static assertion failed");  /* Error: expected 'static_assert' */
    }
    
#ifdef __cplusplus
    /* C++ specific errors */
    
    /* Block 3: RT_DECLTYPE error */
    if (parser_test_condition == 3) {
        /* Missing 'decltype' in trailing return type */
        auto decltype_error() -> etype(parser_test_condition);  /* Error: expected 'decltype' */
    }
    
    /* Block 4: RT_OPERATOR error */
    if (parser_test_condition == 4) {
        struct MyClass {
            int value;
        };
        /* Missing 'operator' in operator overload */
        int +(MyClass a, MyClass b) {  /* Error: expected 'operator' */
            return a.value + b.value;
        }
    }
    
    /* Block 5: RT_CLASS error */
    if (parser_test_condition == 5) {
        /* Missing 'class' keyword in class definition */
        MissingClassKeyword {  /* Error: expected 'class' */
        public:
            int x;
        };
    }
    
    /* Block 6: RT_TEMPLATE error */
    if (parser_test_condition == 6) {
        /* Missing 'template' keyword */
        <typename T> void template_error() {}  /* Error: expected 'template' */
    }
    
    /* Block 7: RT_NAMESPACE error */
    if (parser_test_condition == 7) {
        /* Missing 'namespace' keyword */
        my_namespace {  /* Error: expected 'namespace' */
            int y;
        }
    }
    
    /* Block 8: RT_USING error */
    if (parser_test_condition == 8) {
        /* Missing 'using' in using directive */
        namespace std;  /* Error: expected 'using' */
    }
    
    /* Block 9: RT_TRY error */
    if (parser_test_condition == 9) {
        /* Missing 'try' keyword */
        {  /* Error: expected 'try' */
            throw 42;
        } catch (...) {
        }
    }
#endif
    
    /* Block 10: RT_ASM error - C and C++ */
    if (parser_test_condition == 10) {
        /* Missing 'asm' keyword in inline assembly */
        volatile ("nop");  /* Error: expected 'asm' */
    }
    
    /* Additional tests with attributes and macros */
    
    /* Test with __attribute__ to affect parser state */
    if (parser_test_condition == 11) {
        /* Attribute before missing extern */
        __attribute__((weak)) "C" void attr_extern_error();  /* Error: expected 'extern' */
    }
    
    /* Macro expansion error for RT_CLASS */
    if (parser_test_condition == 12) {
        BAD_ MyMacroClass {};  /* Error after macro expansion: expected 'class' */
    }
    
    /* Nested scope error for RT_OPERATOR */
    if (parser_test_condition == 13) {
#ifdef __cplusplus
        namespace Nested {
            struct Test {
                /* Missing operator in class scope */
                bool ==(const Test& other) const;  /* Error: expected 'operator' */
            };
        }
#endif
    }
    
    /* Complex static_assert error context */
    if (parser_test_condition == 14) {
        /* static_assert in struct/class context */
#ifdef __cplusplus
        struct WithAssert {
            (sizeof(int) == 4, "int must be 4 bytes");  /* Error: expected 'static_assert' */
            int data;
        };
#else
        struct WithAssert {
            (sizeof(int) == 4, "int must be 4 bytes");  /* Error: expected 'static_assert' */
            int data;
        };
#endif
    }
    
    /* Template with missing 'template' in nested context */
    if (parser_test_condition == 15) {
#ifdef __cplusplus
        class Outer {
            /* Missing template inside class */
            <typename U> void inner_method();  /* Error: expected 'template' */
        };
#endif
    }
    
    /* Return valid result */
    return 0;
}

/* Additional test cases outside main function */

/* Test RT_EXTERN at file scope */
#ifdef __cplusplus
extern "C"
#else
extern
#endif
"MissingLinkage" void file_scope_extern_error();  /* Error context varies */

/* Test RT_STATIC_ASSERT at file scope - C11/C++ */
#if __STDC_VERSION__ >= 201112L || defined(__cplusplus)
(1 == 2, "file scope assertion");  /* Error: expected 'static_assert' */
#endif

#ifdef __cplusplus
/* Test RT_NAMESPACE with attributes */
__attribute__((visibility("default"))) 
MissingNamespaceName {  /* Error: expected 'namespace' */
    void function();
}

/* Test RT_USING in different contexts */
class TestClass {
    /* Missing using for inheritance */
    BaseClass;  /* Error: expected 'using' in some contexts */
public:
    void method();
};
#endif

/* Test RT_ASM with volatile qualifier */
void asm_test_function() {
    __volatile__ ("mov %eax, %ebx");  /* Error: expected 'asm' (depends on context) */
}
