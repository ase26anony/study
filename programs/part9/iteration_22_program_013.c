#ifdef __cplusplus
#include <iostream>
#endif

/* Use volatile to prevent dead code elimination */
volatile int parser_mode = 0;

/* Macro to test parser state during error recovery */
#define BAD_CLASS class
#define BAD_TEMPLATE template
#define BAD_NAMESPACE namespace

int main() {
    /* Valid main function that compiles successfully */
    
    /* Each error block is isolated to prevent cascading failures */
    
    /* 1. RT_EXTERN - Missing 'extern' in linkage specification */
    if (parser_mode == 1) {
        /* ERROR: Missing 'extern' */
        "C" void missing_extern_func();
    }
    
    /* 2. RT_STATIC_ASSERT - Missing 'static_assert' keyword */
    if (parser_mode == 2) {
        /* ERROR: Missing 'static_assert' */
        (1 == 1, "static assertion failed");
    }
    
#ifdef __cplusplus
    /* C++ specific errors */
    
    /* 3. RT_DECLTYPE - Misspelled 'decltype' in trailing return */
    if (parser_mode == 3) {
        auto missing_decltype_func() -> etype(1 + 2);
    }
    
    /* 4. RT_OPERATOR - Missing 'operator' in overload */
    if (parser_mode == 4) {
        struct MyClass {
            int value;
        };
        /* ERROR: Missing 'operator' */
        int +(MyClass a, MyClass b) {
            return a.value + b.value;
        }
    }
    
    /* 5. RT_CLASS - Missing 'class' in definition */
    if (parser_mode == 5) {
        /* ERROR: Missing 'class' */
        MissingClassDef {
        public:
            int x;
        };
    }
    
    /* 6. RT_TEMPLATE - Missing 'template' keyword */
    if (parser_mode == 6) {
        /* ERROR: Missing 'template' */
        <typename T>
        void missing_template_func(T t) {}
    }
    
    /* 7. RT_NAMESPACE - Missing 'namespace' keyword */
    if (parser_mode == 7) {
        /* ERROR: Missing 'namespace' */
        missing_ns {
            int x;
        }
    }
    
    /* 8. RT_USING - Missing 'using' in directive */
    if (parser_mode == 8) {
        /* ERROR: Missing 'using' */
        namespace std;
    }
    
    /* 9. RT_TRY - Missing 'try' in try-catch */
    if (parser_mode == 9) {
        /* ERROR: Missing 'try' */
        {
            throw 42;
        }
        catch (...) {
        }
    }
#endif

    /* 10. RT_ASM - Missing 'asm' in inline assembly */
    if (parser_mode == 10) {
        /* ERROR: Missing 'asm' */
        volatile ("nop");
    }
    
    /* Test with macro expansions to affect parser state */
    if (parser_mode == 11) {
        /* ERROR: Macro expands to incomplete token */
        BAD_ IncompleteClass {};
    }
    
    /* Test with attributes */
    if (parser_mode == 12) {
        /* ERROR: Missing 'class' after attribute */
        __attribute__((packed))
        StructAfterAttr {
            int x;
        };
    }
    
    /* Nested scope test */
    if (parser_mode == 13) {
        void nested_function() {
            /* ERROR: Missing 'extern' inside function */
            "C" void nested_extern();
        }
    }
    
    /* Multiple errors in different scopes */
    if (parser_mode == 14) {
        namespace outer {
            /* ERROR: Missing 'class' inside namespace */
            ClassInNamespace {
                void method() {
                    /* ERROR: Missing 'try' inside method */
                    {
                        throw 1;
                    }
                    catch (...) {}
                }
            };
        }
    }
    
    return 0;
}

/* Additional test cases outside main */

/* Static assert in C mode */
#ifdef __cplusplus
#else
/* C11 static_assert */
/* ERROR: Missing 'static_assert' in C mode */
_Static_assert(1 == 1, "C static assert");
#endif

/* Linkage specification at file scope */
/* ERROR: Missing 'extern' at file scope */
"C" int file_scope_var;

#ifdef __cplusplus
/* Template at file scope */
/* ERROR: Missing 'template' at file scope */
<typename T>
T template_func(T x) { return x; }

/* Using directive at file scope */
/* ERROR: Missing 'using' at file scope */
namespace boost;

/* Class definition with operator */
/* ERROR: Missing 'operator' in class */
struct TestClass {
    int value;
    int +(const TestClass& other) {
        return value + other.value;
    }
};
#endif
