/* Test program to trigger specific parser error recovery paths */
#ifdef __cplusplus
#include <iostream>
#endif

/* Volatile control variable to ensure all code paths are parsed */
static volatile int parser_test_condition = 0;

/* Macro-based error to test parser state during macro expansion */
#define EXPECT_CLASS class
#define EXPECT_TEMPLATE template
#define EXPECT_NAMESPACE namespace

int main(void) {
    /* Main function is valid and compilable */
    
    /* Use volatile to control which error blocks are parsed */
    volatile int test_case = 0;
    
    /* ===== RT_EXTERN ===== */
    /* C and C++: Missing 'extern' in linkage specification */
    if (parser_test_condition) {
        /* C mode error */
        /* Missing 'extern' before "C" */
        "C" void missing_extern_func(void);
    }
    
    /* ===== RT_STATIC_ASSERT ===== */
    /* C11/C++11: Missing 'static_assert' keyword */
    if (test_case == 1) {
        /* Invalid: static_assert without keyword */
        (1 == 1, "static_assert test");
        
#ifdef __cplusplus
        /* C++ version */
        static_assert /* missing keyword test */ (1 == 1);
#endif
    }
    
#ifdef __cplusplus
    /* C++-specific error cases */
    
    /* ===== RT_DECLTYPE ===== */
    /* Missing 'decltype' in trailing return type */
    if (test_case == 2) {
        auto missing_decltype_func() -> /* missing decltype */ (x + y);
    }
    
    /* ===== RT_OPERATOR ===== */
    /* Missing 'operator' in operator overload */
    if (test_case == 3) {
        struct TestClass {
            int value;
        };
        
        /* Should be: TestClass operator+(TestClass a, TestClass b) */
        TestClass +(TestClass a, TestClass b) {
            return {a.value + b.value};
        }
    }
    
    /* ===== RT_CLASS ===== */
    /* Missing 'class' in class definition */
    if (test_case == 4) {
        /* Direct error */
        MissingClassKeyword {
            int x;
        };
        
        /* Macro-based error */
        EXPECT_ MissingClass {
            int y;
        };
    }
    
    /* ===== RT_TEMPLATE ===== */
    /* Missing 'template' keyword */
    if (test_case == 5) {
        /* Direct error */
        <typename T>
        void missing_template_func(T t) {}
        
        /* Macro-based with attribute */
        __attribute__((unused))
        <int N>
        void another_missing_template() {}
    }
    
    /* ===== RT_NAMESPACE ===== */
    /* Missing 'namespace' keyword */
    if (test_case == 6) {
        /* Direct error */
        MyNamespace {
            int value;
        }
        
        /* Macro-based error */
        EXPECT_ MyOtherNamespace {
            void func();
        }
    }
    
    /* ===== RT_USING ===== */
    /* Missing 'using' in using directive */
    if (test_case == 7) {
        /* Missing 'using' before namespace */
        namespace std;
        
        /* In function scope */
        void test_func() {
            namespace boost;
        }
    }
    
    /* ===== RT_TRY ===== */
    /* Missing 'try' in try-catch block */
    if (test_case == 8) {
        /* Missing 'try' keyword */
        {
            throw 42;
        }
        catch (...) {
            /* handler */
        }
        
        /* Nested in function */
        void throwing_func() {
            {
                throw "error";
            }
            catch (const char* e) {}
        }
    }
#endif
    
    /* ===== RT_ASM ===== */
    /* Missing 'asm' in inline assembly (C and C++) */
    if (test_case == 9) {
        /* Missing 'asm' keyword */
        volatile (
            "nop"
        );
        
        /* With qualifiers but missing 'asm' */
        __volatile__ (
            "mov %0, %1"
            :
            : "r" (test_case)
        );
    }
    
    /* Additional C-specific tests */
#ifndef __cplusplus
    /* C mode specific tests */
    if (test_case == 10) {
        /* _Static_assert missing in C11 */
        _Static_assert /* test missing keyword */ (1, "fail");
    }
#endif
    
    /* Test with attributes affecting parser state */
    if (test_case == 11) {
        __attribute__((unused, deprecated))
        /* This creates a parsing context where certain tokens are expected */
        extern /* parser expects something after attribute */ ;
    }
    
    /* Nested error in block scope */
    {
        volatile int inner_test = 0;
        if (inner_test) {
            /* Another extern error in nested scope */
            "C++" void nested_extern_error(void);
        }
    }
    
    /* Final valid return */
    return 0;
}

#ifdef __cplusplus
/* Additional C++ namespace context errors */
namespace Outer {
    /* Missing 'namespace' in nested namespace (C++17) */
    Inner {
        int value;
    }
    
    /* Missing 'class' inside namespace */
    class OuterClass {
        /* Missing 'class' for nested class */
        NestedClass {
            int x;
        };
    };
    
    /* Template with missing 'template' in namespace */
    <typename T>
    T namespace_template_func(T t) { return t; }
}

/* Test in global scope with attributes */
__attribute__((visibility("default")))
/* Missing 'extern' after attribute */
"C" void attributed_extern_error(void);

/* Complex case: multiple expected tokens */
template<typename T>
class ComplexCase {
    /* Missing 'operator' inside template class */
    bool ==(const ComplexCase& other) const {
        return true;
    }
    
    /* Missing 'template' for member template */
    <typename U>
    void member_template() {}
};
#endif

/* C-specific global scope errors */
#ifndef __cplusplus
/* Missing 'extern' at file scope */
"C" int c_global_var = 42;

/* Missing 'asm' at file scope */
volatile ("nop");
#endif
