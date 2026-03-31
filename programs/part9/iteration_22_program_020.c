/* Test program to trigger specific parser error recovery paths */
#ifdef __cplusplus
#include <iostream>
#endif

/* Volatile control variable to prevent dead code elimination */
volatile int parser_test_mode = 0;

/* Macro-based error to test parser state during macro expansion */
#define BAD_CLASS class
#define BAD_EXTERN extern
#define BAD_ASM asm

int main() {
    /* Valid main function that will compile successfully */
    
    /* Use volatile to control which error blocks are parsed */
    if (parser_test_mode == 0) {
        /* RT_EXTERN error in C context */
        /* Missing 'extern' in linkage specification */
        "C" void missing_extern_func();  /* Error: expected 'extern' */
    }
    
    if (parser_test_mode == 1) {
        /* RT_STATIC_ASSERT error */
        /* Missing 'static_assert' keyword */
        (1 == 1, "static assertion failed");  /* Error: expected 'static_assert' */
    }

#ifdef __cplusplus
    /* C++ specific errors */
    
    if (parser_test_mode == 2) {
        /* RT_DECLTYPE error */
        /* Misspelled decltype in trailing return type */
        auto decltype_error() -> etype(parser_test_mode);  /* Error: expected 'decltype' */
    }
    
    if (parser_test_mode == 3) {
        /* RT_OPERATOR error */
        /* Missing 'operator' in operator overload */
        struct MyClass {
            int value;
        };
        int +(MyClass a, MyClass b) {  /* Error: expected 'operator' */
            return a.value + b.value;
        }
    }
    
    if (parser_test_mode == 4) {
        /* RT_CLASS error - using macro expansion */
        /* Macro expands to incomplete token */
        BAD_ MyType {  /* Error: expected 'class' (after macro expansion) */
            int x;
        };
    }
    
    if (parser_test_mode == 5) {
        /* RT_CLASS error - direct */
        /* Missing 'class' keyword */
        MissingClassKeyword {  /* Error: expected 'class' */
        public:
            int member;
        };
    }
    
    if (parser_test_mode == 6) {
        /* RT_TEMPLATE error */
        /* Missing 'template' keyword */
        <typename T>  /* Error: expected 'template' */
        void template_missing() {}
    }
    
    if (parser_test_mode == 7) {
        /* RT_TEMPLATE error with attribute */
        /* Template after attribute, missing keyword */
        __attribute__((deprecated))
        <int N>  /* Error: expected 'template' */
        struct AttributedTemplate {};
    }
    
    if (parser_test_mode == 8) {
        /* RT_NAMESPACE error */
        /* Missing 'namespace' keyword */
        my_namespace {  /* Error: expected 'namespace' */
            int x;
        }
    }
    
    if (parser_test_mode == 9) {
        /* RT_USING error */
        /* Missing 'using' keyword */
        namespace std;  /* Error: expected 'using' */
    }
    
    if (parser_test_mode == 10) {
        /* RT_TRY error */
        /* Missing 'try' keyword */
        {  /* Error: expected 'try' */
            throw 42;
        } catch (...) {
            // Handle exception
        }
    }
    
    /* Nested scope test */
    namespace outer {
        if (parser_test_mode == 11) {
            /* RT_CLASS error inside namespace */
            InnerClass {  /* Error: expected 'class' */
                int data;
            };
        }
    }
    
    /* Function scope test */
    void test_function() {
        if (parser_test_mode == 12) {
            /* RT_DECLTYPE error inside function */
            auto local_error() -> declltype(parser_test_mode);  /* Error: expected 'decltype' */
        }
    }
#endif  /* __cplusplus */

    /* C-compatible errors */
    if (parser_test_mode == 13) {
        /* RT_ASM error - using macro */
        /* Missing 'asm' keyword via macro */
        BAD_ volatile ("nop");  /* Error: expected 'asm' */
    }
    
    if (parser_test_mode == 14) {
        /* RT_ASM error - direct */
        /* Missing 'asm' keyword */
        volatile ("mov %eax, %ebx");  /* Error: expected 'asm' */
    }
    
    if (parser_test_mode == 15) {
        /* RT_EXTERN error with attribute */
        /* Missing 'extern' after attribute */
        __attribute__((weak))
        "C++"  /* Error: expected 'extern' */
        void attributed_extern_func();
    }
    
    /* Additional static_assert test for C11/C++ */
    if (parser_test_mode == 16) {
        /* RT_STATIC_ASSERT in different context */
        struct TestStruct {
            /* Missing static_assert in struct */
            (sizeof(int) == 4, "int must be 4 bytes");  /* Error: expected 'static_assert' */
            int field;
        };
    }

    return 0;
}

/* Additional test cases outside main */

/* RT_EXTERN test at file scope */
"C" void file_scope_extern_error();  /* Error: expected 'extern' */

#ifdef __cplusplus
/* RT_NAMESPACE test at file scope */
global_namespace_error {  /* Error: expected 'namespace' */
    int global_var;
}

/* RT_CLASS with inheritance error */
class Base {};
Derived : public Base {  /* Error: expected 'class' */
    int derived_member;
};

/* RT_OPERATOR as member function */
struct OperatorTest {
    int operator+(const OperatorTest& other);  /* Valid */
    int +(const OperatorTest& other);  /* Error: expected 'operator' */
};
#endif

/* RT_ASM in function */
void asm_function() {
    volatile ("nop");  /* Error: expected 'asm' */
}
