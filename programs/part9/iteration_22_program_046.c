/* Test program to trigger specific parser error recovery paths */
#ifdef __cplusplus
#include <iostream>
#endif

volatile int parser_test_condition = 0;

int main() {
    /* Valid main function that compiles successfully */
    return 0;
}

/* ===== C/C++ COMMON TOKENS ===== */

/* RT_EXTERN: Missing 'extern' in linkage specification */
#if 0
void test_extern_error() {
    /* Missing 'extern' before "C" */
    "C" void linkage_func(void);  /* Expected: extern "C" */
}
#endif

/* RT_STATIC_ASSERT: Missing 'static_assert' keyword */
#if 0
void test_static_assert_error() {
    /* Missing static_assert keyword */
    (1 == 1, "Assertion failed");  /* Expected: static_assert(1 == 1, "...") */
}
#endif

/* RT_ASM: Missing 'asm' keyword for inline assembly */
#if 0
void test_asm_error() {
    int result;
    /* Missing 'asm' keyword */
    volatile ("mov %0, %1" : "=r"(result) : "r"(42));  /* Expected: asm volatile(...) */
}
#endif

/* ===== C++ ONLY TOKENS ===== */
#ifdef __cplusplus

/* RT_DECLTYPE: Missing 'decltype' in trailing return type */
#if 0
auto test_decltype_error() -> /* Missing decltype */ (x) {
    int x = 5;
    return x;
}
#endif

/* RT_OPERATOR: Missing 'operator' in overload definition */
#if 0
class MyClass {
    int value;
public:
    MyClass(int v) : value(v) {}
};

/* Missing 'operator' keyword */
int +(MyClass a, MyClass b) {  /* Expected: operator+ */
    return a.value + b.value;
}
#endif

/* RT_CLASS: Missing 'class' keyword in definition */
#if 0
/* Missing 'class' keyword */
MissingClassKeyword {  /* Expected: class MissingClassKeyword */
    int member;
};
#endif

/* RT_TEMPLATE: Missing 'template' keyword */
#if 0
/* Missing 'template' keyword */
<typename T>  /* Expected: template<typename T> */
void template_func(T value) {}
#endif

/* RT_NAMESPACE: Missing 'namespace' keyword */
#if 0
/* Missing 'namespace' keyword */
MyNamespace {  /* Expected: namespace MyNamespace */
    int value = 42;
}
#endif

/* RT_USING: Missing 'using' keyword */
#if 0
/* Missing 'using' keyword */
namespace std;  /* Expected: using namespace std; */
#endif

/* RT_TRY: Missing 'try' keyword */
#if 0
void test_try_error() {
    /* Missing 'try' keyword */
    {  /* Expected: try { */
        throw 42;
    } catch (int e) {
        // Handle exception
    }
}
#endif

/* ===== MACRO-BASED ERRORS ===== */

/* RT_CLASS triggered through macro expansion */
#if 0
#define BAD_CLASS_DEF class
BAD_ MyMacroClass {  /* Missing 'class' from macro expansion */
    int x;
};
#endif

/* RT_TEMPLATE triggered through macro with attributes */
#if 0
#define PROBLEMATIC_TEMPLATE template
__attribute__((always_inline))
<typename U>  /* Missing 'template' due to attribute interaction */
void attributed_func(U u) {}
#endif

/* ===== VOLATILE-CONTROLLED ERRORS ===== */

/* Conditional parsing based on volatile variable */
#if 0
void volatile_controlled_errors() {
    if (parser_test_condition) {
        /* RT_OPERATOR error in conditional block */
        bool ==(MyClass a, MyClass b) {  /* Missing 'operator' */
            return a.value == b.value;
        }
    }
    
    if (!parser_test_condition) {
        /* RT_USING error in another conditional block */
        namespace boost;  /* Missing 'using' */
    }
}
#endif

/* ===== NESTED SCOPE ERRORS ===== */

#if 0
namespace OuterNamespace {
    /* RT_CLASS error inside namespace */
    class Outer {
    public:
        /* RT_OPERATOR error inside class method context */
        void method() {
            /* Missing 'operator' inside class method */
            int *(MyClass a, MyClass b);  /* Expected: operator* */
        }
        
        /* RT_TEMPLATE error in nested template context */
        template<typename T>
        void template_method() {
            /* Missing 'template' in dependent context */
            <typename U>  /* Expected: template<typename U> */
            void nested_template() {}
        }
    };
}
#endif

#endif /* __cplusplus */

/* ===== C-SPECIFIC VARIANTS ===== */
#ifndef __cplusplus

/* RT_EXTERN in C mode */
#if 0
/* Missing 'extern' in C */
"C" {  /* Expected: extern "C" */
    void c_func(void);
}
#endif

/* RT_STATIC_ASSERT in C11 mode */
#if 0
/* Missing _Static_assert in C */
(1 == 1, "C assertion");  /* Expected: _Static_assert(1 == 1, "...") */
#endif

/* RT_ASM with GNU C extensions */
#if 0
void c_asm_error() {
    int x;
    /* Missing 'asm' in C */
    __volatile__ ("movl $1, %0" : "=r"(x));  /* Expected: asm __volatile__(...) */
}
#endif

#endif /* !__cplusplus */
