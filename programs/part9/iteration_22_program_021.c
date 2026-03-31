/* Test program to trigger uncovered error recovery blocks in gcc/parser.cc
   Lines 34457-34486: Expected token diagnostic messages */
   
#ifdef __cplusplus
#include <type_traits>
#endif

/* Volatile control variable to prevent dead code elimination */
static volatile int parser_test_phase = 0;

/* Macro-based error tests */
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

/* Test 1: RT_EXTERN - Missing 'extern' in linkage specification */
void test_extern_error(void) {
    if (parser_test_phase == EXPECT_EXTERN_ERROR) {
        /* Missing 'extern' before "C" */
        "C" void linkage_spec_func(void);  /* Should trigger: expected 'extern' */
        
        /* Also test with attributes */
        __attribute__((weak)) "C" int attr_linkage(void);
    }
}

/* Test 2: RT_STATIC_ASSERT - Missing 'static_assert' keyword */
void test_static_assert_error(void) {
    if (parser_test_phase == EXPECT_STATIC_ASSERT_ERROR) {
        /* C++ static_assert without keyword */
        #ifdef __cplusplus
        (sizeof(int) == 4, "int must be 4 bytes");  /* Should trigger: expected 'static_assert' */
        #else
        /* C11 _Static_assert test */
        _Static_assert(1, "C static assert");  /* Valid for comparison */
        #endif
        
        /* In macro expansion */
        #define MY_ASSERT(cond, msg) (cond, msg)
        MY_ASSERT(1, "fail");
    }
}

#ifdef __cplusplus
/* Test 3: RT_DECLTYPE - Missing 'decltype' in type specifier */
void test_decltype_error(void) {
    if (parser_test_phase == EXPECT_DECLTYPE_ERROR) {
        /* Missing 'decltype' in trailing return type */
        auto decltype_test() -> etype(1 + 2);  /* Should trigger: expected 'decltype' */
        
        /* In template context */
        template<typename T>
        typename T::type get_type();
        
        /* Using misspelled decltype */
        auto x = etype(5 + 3);
    }
}

/* Test 4: RT_OPERATOR - Missing 'operator' in overload */
struct MyClass {
    int value;
};

void test_operator_error(void) {
    if (parser_test_phase == EXPECT_OPERATOR_ERROR) {
        /* Missing 'operator' keyword */
        int +(MyClass a, MyClass b) {  /* Should trigger: expected 'operator' */
            return a.value + b.value;
        }
        
        /* Conversion operator without keyword */
        int() { return 42; }  /* In class context would trigger */
    }
}

/* Test 5: RT_CLASS - Missing 'class' in definition */
void test_class_error(void) {
    if (parser_test_phase == EXPECT_CLASS_ERROR) {
        /* Missing 'class' keyword */
        MissingClassKeyword {  /* Should trigger: expected 'class' */
            int x;
        };
        
        /* After attributes */
        __attribute__((aligned(16))) BadClass {
            double data;
        };
    }
}

/* Test 6: RT_TEMPLATE - Missing 'template' keyword */
void test_template_error(void) {
    if (parser_test_phase == EXPECT_TEMPLATE_ERROR) {
        /* Missing 'template' before angle brackets */
        <typename T>  /* Should trigger: expected 'template' */
        void template_func(T t) {}
        
        /* Template class without keyword */
        <class T> class TemplateClass;
    }
}

/* Test 7: RT_NAMESPACE - Missing 'namespace' keyword */
void test_namespace_error(void) {
    if (parser_test_phase == EXPECT_NAMESPACE_ERROR) {
        /* Missing 'namespace' */
        my_namespace {  /* Should trigger: expected 'namespace' */
            int value;
        }
        
        /* Inline namespace without keyword */
        inline my_inline_ns { }
    }
}

/* Test 8: RT_USING - Missing 'using' keyword */
void test_using_error(void) {
    if (parser_test_phase == EXPECT_USING_ERROR) {
        /* Missing 'using' in directive */
        namespace std;  /* Should trigger: expected 'using' */
        
        /* Using declaration without keyword */
        std::cout;
    }
}

/* Test 9: RT_TRY - Missing 'try' keyword */
void test_try_error(void) {
    if (parser_test_phase == EXPECT_TRY_ERROR) {
        /* Missing 'try' before block */
        {  /* Should trigger: expected 'try' */
            throw 42;
        }
        catch (...) {
        }
        
        /* Function-try-block without try */
        void func()
        {  /* Should trigger */
            throw 1;
        }
        catch(...) { }
    }
}
#endif /* __cplusplus */

/* Test 10: RT_ASM - Missing 'asm' keyword (works in C and C++) */
void test_asm_error(void) {
    if (parser_test_phase == EXPECT_ASM_ERROR) {
        /* Missing 'asm' in inline assembly */
        volatile ("nop");  /* Should trigger: expected 'asm' */
        
        /* With qualifiers but no asm */
        __volatile__ ("mov %eax, %ebx");
    }
}

/* Additional C-specific tests */
#ifndef __cplusplus
/* C version of static_assert error */
void test_c_static_assert_error(void) {
    if (parser_test_phase == EXPECT_STATIC_ASSERT_ERROR) {
        /* Missing '_Static_assert' in C11 */
        (1, "C static assert failed");  /* Should trigger error */
    }
}
#endif

/* Main function - valid code that controls which error is parsed */
int main(void) {
    /* Each test is guarded by volatile condition */
    /* Only one will be parsed at a time based on parser_test_phase */
    
    test_extern_error();
    test_static_assert_error();
    
    #ifdef __cplusplus
    test_decltype_error();
    test_operator_error();
    test_class_error();
    test_template_error();
    test_namespace_error();
    test_using_error();
    test_try_error();
    #else
    test_c_static_assert_error();
    #endif
    
    test_asm_error();
    
    return 0;
}

/* Additional edge cases in different scopes */
#ifdef __cplusplus
namespace Outer {
    /* Nested namespace error */
    void nested_namespace_test() {
        if (parser_test_phase == EXPECT_NAMESPACE_ERROR) {
            Inner {  /* Missing 'namespace' inside another namespace */
                int x;
            }
        }
    }
    
    /* Class member with template error */
    class Container {
        void member_func() {
            if (parser_test_phase == EXPECT_TEMPLATE_ERROR) {
                <typename U>  /* Missing 'template' inside class */
                void nested_template() {}
            }
        }
    };
}
#endif

/* Macro-based token omission */
#define OMIT_EXTERN extern
#define OMIT_CLASS class

void macro_based_errors(void) {
    /* These might trigger different parser states */
    #ifdef __cplusplus
    if (parser_test_phase == EXPECT_CLASS_ERROR) {
        OMIT_ MyMacroClass {  /* Macro expansion missing token */
            int y;
        };
    }
    #endif
    
    if (parser_test_phase == EXPECT_EXTERN_ERROR) {
        OMIT_ "C++" {  /* Missing extern from macro */
            void func(void);
        };
    }
}
