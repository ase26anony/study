/* Test program to trigger specific parser error recovery paths */
#ifdef __cplusplus
#include <iostream>
#endif

/* Volatile control variable to prevent dead code elimination */
static volatile int parser_test_mode = 0;

/* Macro-based error triggers */
#define EXPECT_EXTERN_ERROR 1
#define EXPECT_STATIC_ASSERT_ERROR 2
#define EXPECT_DECLTYPE_ERROR 3
#define EXPECT_OPERATOR_ERROR 4
#define EXPECT_CLASS_ERROR 5
#define EXPECT_TEMPLATE_ERROR 6
#define EXPECT_NAMESPACE_ERROR 7
#define EXPECT_USING_ERROR 8
#define EXPECT_ASM_ERROR 9
#define EXPECT_TRY_ERROR 10

/* Preprocessor trick to hide keywords */
#define HIDE_KEYWORD(x) x
#define BAD_TEMPLATE template
#define BAD_CLASS class

int main() {
    /* Valid main function that will compile */
    int result = 0;
    
    /* Use volatile to control which error paths are parsed */
    if (parser_test_mode == EXPECT_EXTERN_ERROR) {
        /* RT_EXTERN: Missing 'extern' in linkage specification - C and C++ */
        /* In C mode */
#ifndef __cplusplus
        "C" void missing_extern_func(void);  /* Error: expected 'extern' */
#else
        extern "C" {
            /* Valid in C++ to contrast */
            void valid_func(void);
        }
        /* Error case in C++ */
        "C++" void another_missing_extern(void);  /* Error: expected 'extern' */
#endif
    }
    
    if (parser_test_mode == EXPECT_STATIC_ASSERT_ERROR) {
        /* RT_STATIC_ASSERT: Missing 'static_assert' keyword - C11 and C++11 */
        /* In C mode */
#ifndef __cplusplus
        _Static_assert(1, "Valid C11 static assert");
        /* Error: missing static_assert keyword */
        (1, "Missing static_assert");  /* Error: expected 'static_assert' */
#else
        static_assert(1 == 1, "Valid C++ static assert");
        /* Error case */
        (sizeof(int) == 4, "Missing static_assert keyword");  /* Error: expected 'static_assert' */
#endif
    }
    
#ifdef __cplusplus
    /* C++ only tokens */
    if (parser_test_mode == EXPECT_DECLTYPE_ERROR) {
        /* RT_DECLTYPE: Missing 'decltype' in trailing return type */
        auto missing_decltype_func() -> etype(5 + 3) {  /* Error: expected 'decltype' */
            return 5 + 3;
        }
    }
    
    if (parser_test_mode == EXPECT_OPERATOR_ERROR) {
        /* RT_OPERATOR: Missing 'operator' in operator overload */
        struct TestStruct {
            int value;
        };
        
        /* Error: missing 'operator' keyword */
        int +(TestStruct a, TestStruct b) {  /* Error: expected 'operator' */
            return a.value + b.value;
        }
        
        /* Valid for contrast */
        int operator+(TestStruct a, TestStruct b) {
            return a.value + b.value;
        }
    }
    
    if (parser_test_mode == EXPECT_CLASS_ERROR) {
        /* RT_CLASS: Missing 'class' in class definition */
        /* Using macro to hide keyword */
        BAD_ MissingClass {  /* Error: expected 'class' (after macro expansion) */
            int x;
            void method();
        };
        
        /* Direct error */
        MyClass {  /* Error: expected 'class' */
            int x;
        };
    }
    
    if (parser_test_mode == EXPECT_TEMPLATE_ERROR) {
        /* RT_TEMPLATE: Missing 'template' keyword */
        /* Error: missing template keyword */
        <typename T>  /* Error: expected 'template' */
        void template_missing_func(T x) {
            return;
        }
        
        /* With attributes (testing parser state) */
        __attribute__((always_inline))
        <int N>  /* Error: expected 'template' */
        void another_missing_template() {}
    }
    
    if (parser_test_mode == EXPECT_NAMESPACE_ERROR) {
        /* RT_NAMESPACE: Missing 'namespace' keyword */
        /* Error: missing namespace keyword */
        my_namespace {  /* Error: expected 'namespace' */
            int x = 42;
        }
        
        /* Nested in valid namespace */
        namespace outer {
            inner {  /* Error: expected 'namespace' */
                int y;
            }
        }
    }
    
    if (parser_test_mode == EXPECT_USING_ERROR) {
        /* RT_USING: Missing 'using' keyword */
        /* Error: missing using keyword */
        namespace std;  /* Error: expected 'using' */
        
        /* In a valid context */
        using std::cout;
        namespace std;  /* Error: expected 'using' (for using directive) */
    }
    
    if (parser_test_mode == EXPECT_TRY_ERROR) {
        /* RT_TRY: Missing 'try' keyword */
        /* Error: missing try */
        {  /* Error: expected 'try' */
            throw 42;
        } catch (int e) {
            /* handle */
        }
        
        /* With function context */
        void test_func() {
            {  /* Error: expected 'try' */
                throw "error";
            } catch (const char* e) {
            }
        }
    }
#endif /* __cplusplus */
    
    if (parser_test_mode == EXPECT_ASM_ERROR) {
        /* RT_ASM: Missing 'asm' keyword - C and C++ */
        /* Error: missing asm keyword */
        __volatile__ ("nop");  /* Error: expected 'asm' */
        
        /* Valid for contrast */
        asm volatile ("nop");
        
        /* With gcc-style asm */
        __asm__ volatile ("nop");  /* Valid */
        volatile ("nop");  /* Error: expected 'asm' */
    }
    
    /* Additional C-specific tests */
#ifndef __cplusplus
    if (parser_test_mode == EXPECT_EXTERN_ERROR) {
        /* Another C-specific extern error */
        __attribute__((weak))
        "C"  /* Error: expected 'extern' */
        int c_missing_extern(void);
    }
#endif
    
    return result;
}

/* Additional test functions in different scopes */
#ifdef __cplusplus
namespace TestScope {
    /* Nested error to test scoped parsing */
    void nested_test() {
        if (parser_test_mode == EXPECT_TEMPLATE_ERROR) {
            /* Error in function scope */
            <class T>  /* Error: expected 'template' */
            void nested_template_error() {}
        }
    }
    
    /* Class with error in method */
    class Container {
    public:
        void method_with_error() {
            if (parser_test_mode == EXPECT_OPERATOR_ERROR) {
                /* Error in class method */
                int *(Container a, Container b) {  /* Error: expected 'operator' */
                    return nullptr;
                }
            }
        }
    };
}
#endif

/* Attribute interaction tests */
#ifdef __GNUC__
__attribute__((constructor))
#endif
void init_parser_tests(void) {
    /* This might be parsed early */
    if (parser_test_mode == EXPECT_STATIC_ASSERT_ERROR) {
#ifndef __cplusplus
        /* C version with attribute */
        __attribute__((unused))
        (1, "Attribute before error");  /* Error: expected 'static_assert' */
#endif
    }
}
