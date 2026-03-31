/* Test program to trigger uncovered error recovery paths in gcc/parser.cc */
/* Compile with: g++ -xc++ -fsyntax-only -O0 -fdiagnostics-parseable-fixits */

#include <stdio.h>

/* Volatile control variable to prevent dead code elimination */
static volatile int test_case = 0;

/* Macro to test parser state during error recovery */
#define BAD_CLASS class
#define BAD_TEMPLATE template

int main() {
    /* Valid main function that will compile */
    printf("Parser error recovery test\n");
    
    /* Use volatile to control which error paths are parsed */
    if (test_case == 1) {
        /* RT_EXTERN: Missing 'extern' in linkage specification */
        /* C and C++ compatible */
        "C" void missing_extern_func();  /* Error: expected 'extern' */
    }
    
    if (test_case == 2) {
        /* RT_STATIC_ASSERT: Missing 'static_assert' keyword */
        /* C11 and C++11 onwards */
        #ifdef __cplusplus
        (1 == 1, "static assertion failed");  /* Error: expected 'static_assert' */
        #else
        _Static_assert(1 == 1, "fail");  /* Valid C11, test with omission */
        #endif
    }
    
    /* C++ specific errors */
    #ifdef __cplusplus
    
    if (test_case == 3) {
        /* RT_DECLTYPE: Misspelled decltype in trailing return type */
        struct TestStruct { int x; };
        auto missing_decltype_func() -> etype(TestStruct::x);  /* Error: expected 'decltype' */
    }
    
    if (test_case == 4) {
        /* RT_OPERATOR: Missing 'operator' in overload definition */
        class MyClass {
            int value;
        public:
            MyClass(int v) : value(v) {}
        };
        
        int +(MyClass a, MyClass b) {  /* Error: expected 'operator' */
            return a.value + b.value;
        }
    }
    
    if (test_case == 5) {
        /* RT_CLASS: Missing 'class' keyword in definition */
        /* Using macro to test parser state */
        BAD_ MissingClass {  /* Error: expected 'class' (after macro expansion) */
            int member;
        };
    }
    
    if (test_case == 6) {
        /* RT_TEMPLATE: Missing 'template' keyword */
        /* Inside function scope */
        <typename T>  /* Error: expected 'template' */
        void template_func() {
            T value;
        }
    }
    
    if (test_case == 7) {
        /* RT_NAMESPACE: Missing 'namespace' keyword */
        /* With attributes to test parser state */
        __attribute__((deprecated))
        my_namespace {  /* Error: expected 'namespace' */
            int value;
        }
    }
    
    if (test_case == 8) {
        /* RT_USING: Missing 'using' in directive */
        namespace std {
            template<typename T> class vector {};
        }
        
        namespace std;  /* Error: expected 'using' */
    }
    
    if (test_case == 9) {
        /* RT_TRY: Missing 'try' keyword */
        /* Nested in function */
        void test_try() {
            {  /* Error: expected 'try' */
                throw 42;
            } catch (...) {
                printf("Caught\n");
            }
        }
    }
    
    #endif  /* __cplusplus */
    
    if (test_case == 10) {
        /* RT_ASM: Missing 'asm' keyword in inline assembly */
        /* C and C++ compatible */
        __volatile__ ("nop");  /* Error: expected 'asm' (GCC extended asm) */
    }
    
    /* Additional test with nested contexts */
    #ifdef __cplusplus
    namespace Outer {
        if (test_case == 11) {
            /* RT_CLASS in namespace context */
            struct OuterClass {
                /* Missing 'class' inside class method */
                void method() {
                    Inner  /* Error: expected 'class' or other specifier */
                    {
                        int x;
                    };
                }
            };
        }
    }
    
    /* Test with template context */
    if (test_case == 12) {
        template<typename T>
        class TemplateTest {
            /* Missing 'template' in dependent context */
            friend <typename U>  /* Error: expected 'template' */
            void friend_func(TemplateTest<U>&);
        };
    }
    #endif
    
    return 0;
}

/* Standalone errors in different scopes */
#ifdef __cplusplus
/* RT_OPERATOR at file scope (but invalid) */
int &(MyClass);  /* Error: expected 'operator' */

/* RT_USING at file scope */
namespace missing_using {  /* Actually valid namespace definition */
    int x;
}
#endif

/* RT_EXTERN at file scope with attributes */
__attribute__((weak))
"C" int attribute_extern_func(void);  /* Error: expected 'extern' */

/* RT_STATIC_ASSERT at file scope in C++ mode */
#ifdef __cplusplus
static_assert ;  /* Error: expected '(' after 'static_assert' */
/* The parser may first expect 'static_assert' token if we create a different error */
int trigger_static_assert_error = (1, "test");  /* This might trigger different error */
#endif
