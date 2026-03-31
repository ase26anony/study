/* Test program to trigger specific parser error recovery paths in GCC */
#ifdef __cplusplus
#include <iostream>
#endif

/* Volatile control variable to ensure all code paths are parsed */
volatile int parse_control = 0;

/* Macro-based errors to test parser state machine */
#define BAD_EXTERN extern
#define BAD_CLASS class
#define BAD_TEMPLATE template

int main() {
    /* Main function is valid and compilable */
    
    /* Use volatile conditions to control which error blocks are parsed */
    if (parse_control == 0) {
        /* RT_EXTERN error in C context */
        /* Missing 'extern' in linkage specification */
        "C" void missing_extern_func(void);
    }
    
    if (parse_control == 1) {
        /* RT_STATIC_ASSERT error */
        /* Missing 'static_assert' keyword */
        (1 == 1, "static assertion failed");
    }
    
#ifdef __cplusplus
    /* C++ specific errors */
    if (parse_control == 2) {
        /* RT_DECLTYPE error */
        /* Misspelled decltype in trailing return type */
        auto decltype_error() -> etype(1 + 2);
    }
    
    if (parse_control == 3) {
        /* RT_OPERATOR error */
        /* Missing 'operator' in overload definition */
        struct MyClass {
            int value;
        };
        int +(MyClass a, MyClass b) { return a.value + b.value; }
    }
    
    if (parse_control == 4) {
        /* RT_CLASS error - using macro expansion */
        /* Macro expands to incomplete token */
        BAD_ IncompleteClass {
            int x;
        };
    }
    
    if (parse_control == 5) {
        /* RT_CLASS error - direct omission */
        /* Missing 'class' keyword */
        DirectClassOmission {
            public:
                int y;
        };
    }
    
    if (parse_control == 6) {
        /* RT_TEMPLATE error */
        /* Missing 'template' keyword */
        <typename T>
        void template_missing() {}
    }
    
    if (parse_control == 7) {
        /* RT_TEMPLATE error with macro */
        BAD_<int> void macro_template_error() {}
    }
    
    if (parse_control == 8) {
        /* RT_NAMESPACE error */
        /* Missing 'namespace' keyword */
        missing_namespace_keyword {
            int z;
        }
    }
    
    if (parse_control == 9) {
        /* RT_USING error */
        /* Missing 'using' keyword */
        namespace std;
    }
    
    if (parse_control == 10) {
        /* RT_TRY error */
        /* Missing 'try' keyword */
        {
            throw 42;
        } catch (...) {
            // Handle exception
        }
    }
#endif
    
    if (parse_control == 11) {
        /* RT_ASM error in C context */
        /* Missing 'asm' keyword in inline assembly */
        __volatile__ ("nop");
    }
    
    /* Additional tests with attributes to affect parser state */
    if (parse_control == 12) {
        /* RT_EXTERN with attribute */
        __attribute__((weak)) "C" void attr_extern_error(void);
    }
    
#ifdef __cplusplus
    if (parse_control == 13) {
        /* RT_CLASS with attribute */
        __attribute__((aligned(16))) AttributeClassError {
            double data;
        };
    }
    
    if (parse_control == 14) {
        /* Nested error in namespace */
        namespace outer {
            /* RT_CLASS error inside namespace */
            InnerClassError {
                float f;
            };
        }
    }
    
    if (parse_control == 15) {
        /* RT_OPERATOR inside class */
        struct Container {
            /* Missing 'operator' */
            int [](int index);
        };
    }
    
    if (parse_control == 16) {
        /* RT_STATIC_ASSERT in C++ mode */
        static_assert_missing (sizeof(int) == 4, "int must be 4 bytes");
    }
#endif

    /* Test static_assert in C11 mode */
    if (parse_control == 17) {
        /* RT_STATIC_ASSERT in C context */
        _Static_assert_missing (1, "C static assert error");
    }
    
    /* Valid return to ensure main compiles */
    return 0;
}

/* Additional error contexts outside main */
#ifdef __cplusplus
/* RT_TEMPLATE error at file scope */
<typename U>
void file_scope_template_error() {}

/* RT_NAMESPACE error with preceding code */
int global_var = 42;
unexpected_namespace {
    int inside_error;
}
#endif

/* RT_EXTERN error at file scope */
"C" int file_scope_extern_error;
