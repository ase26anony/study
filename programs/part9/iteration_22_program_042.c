/* Test program to trigger specific parser error recovery paths */
#ifdef __cplusplus
#include <type_traits>
#endif

/* Use volatile to prevent dead code elimination */
volatile int parser_test_condition = 0;

/* Main function - valid and compilable */
int main(void) {
    /* We'll use different conditions to isolate each error */
    volatile int test_case = 0;
    
    /* RT_EXTERN - C and C++ */
    if (parser_test_condition == 1) {
        /* Missing 'extern' in linkage specification */
        "C" void missing_extern_func(void);
    }
    
    /* RT_STATIC_ASSERT - C11/C++11 and later */
    if (parser_test_condition == 2) {
        /* Missing 'static_assert' keyword */
        #ifdef __cplusplus
        (true, "static assertion failed");
        #else
        (1, "static assertion failed");
        #endif
    }
    
#ifdef __cplusplus
    /* C++ specific tokens */
    
    /* RT_DECLTYPE */
    if (parser_test_condition == 3) {
        /* Missing 'decltype' in trailing return type */
        auto missing_decltype_func() -> etype(1 + 2);
    }
    
    /* RT_OPERATOR */
    if (parser_test_condition == 4) {
        class MyClass {
            int value;
        public:
            MyClass(int v) : value(v) {}
        };
        
        /* Missing 'operator' in operator overload */
        int +(MyClass a, MyClass b) {
            return a.value + b.value;
        }
    }
    
    /* RT_CLASS */
    if (parser_test_condition == 5) {
        /* Missing 'class' keyword in class definition */
        MissingClassKeyword {
        public:
            int x;
        };
    }
    
    /* RT_TEMPLATE */
    if (parser_test_condition == 6) {
        /* Missing 'template' keyword */
        <typename T>
        void missing_template_func(T t) {}
    }
    
    /* RT_NAMESPACE */
    if (parser_test_condition == 7) {
        /* Missing 'namespace' keyword */
        missing_namespace {
            int x;
        }
    }
    
    /* RT_USING */
    if (parser_test_condition == 8) {
        /* Missing 'using' in using directive */
        namespace std;
    }
    
    /* RT_TRY */
    if (parser_test_condition == 9) {
        /* Missing 'try' keyword */
        {
            throw 42;
        } catch (...) {
            /* handle */
        }
    }
#endif

    /* RT_ASM - C and C++ */
    if (parser_test_condition == 10) {
        /* Missing 'asm' keyword for inline assembly */
        #ifdef __GNUC__
        volatile ("nop");
        #endif
    }
    
    /* Additional tests with preprocessor interactions */
    
    /* RT_EXTERN with macro */
    if (parser_test_condition == 11) {
        #define LINKAGE_SPEC "C"
        LINKAGE_SPEC void macro_extern_func(void);
    }
    
    /* RT_CLASS with macro and attributes */
    if (parser_test_condition == 12) {
        #ifdef __cplusplus
        #define CLASS_DEF class
        CLASS_DEF __attribute__((deprecated)) MacroClassWithAttr {
            int y;
        };
        #endif
    }
    
    /* RT_STATIC_ASSERT in different contexts */
    if (parser_test_condition == 13) {
        #ifdef __cplusplus
        template<typename T>
        void template_func() {
            /* Missing static_assert in template */
            (std::is_integral<T>::value, "T must be integral");
        }
        #endif
    }
    
    /* Nested errors in different scopes */
    if (parser_test_condition == 14) {
        #ifdef __cplusplus
        namespace outer {
            /* Missing 'namespace' inside namespace */
            inner {
                /* Missing 'class' inside nested namespace */
                InnerClass {
                    int z;
                };
            }
        }
        #endif
    }
    
    /* RT_OPERATOR in class scope */
    if (parser_test_condition == 15) {
        #ifdef __cplusplus
        class OperatorTest {
            int data;
        public:
            /* Missing 'operator' in conversion operator */
            int() const { return data; }
        };
        #endif
    }
    
    /* RT_TEMPLATE with multiple parameters */
    if (parser_test_condition == 16) {
        #ifdef __cplusplus
        <typename T, typename U, int N>
        class MissingTemplateKeyword {
            T array[N];
        };
        #endif
    }
    
    /* RT_DECLTYPE in complex expression */
    if (parser_test_condition == 17) {
        #ifdef __cplusplus
        auto complex_decltype() -> etype(1 + 2 * 3.0);
        #endif
    }
    
    /* RT_USING with elaborated forms */
    if (parser_test_condition == 18) {
        #ifdef __cplusplus
        /* Missing 'using' in using declaration */
        namespace::some_name;
        
        /* Missing 'using' in using alias */
        my_alias = int;
        #endif
    }
    
    /* RT_ASM with extended syntax */
    if (parser_test_condition == 19) {
        #ifdef __GNUC__
        volatile goto ("nop");
        #endif
    }
    
    /* RT_TRY with different contexts */
    if (parser_test_condition == 20) {
        #ifdef __cplusplus
        void function_with_try() {
            /* Missing 'try' in function try block */
            {
                throw "error";
            } catch (const char* e) {
                /* handle */
            }
        }
        #endif
    }
    
    return 0;
}

/* Additional test cases in global scope with #ifdef guards */

/* RT_EXTERN at file scope - C mode */
#ifndef __cplusplus
/* Missing 'extern' */
"C" {
    int global_c_func(void);
}
#endif

/* RT_STATIC_ASSERT at file scope */
#ifdef __cplusplus
/* Missing 'static_assert' */
(true, "global assertion");
#endif

/* RT_ASM in function with attributes */
#ifdef __GNUC__
void __attribute__((naked)) naked_func(void) {
    /* Missing 'asm' */
    volatile ("ret");
}
#endif

/* Macro that expands to incomplete syntax */
#define BAD_SYNTAX(class) class
#ifdef __cplusplus
BAD_SYNTAX(/* Missing class name */ {
    int member;
});
#endif

/* Test with __extension__ to affect parser state */
#ifdef __GNUC__
__extension__ struct {
    /* Try to trigger error recovery after extension */
    int x;
} ext_struct;
#endif
