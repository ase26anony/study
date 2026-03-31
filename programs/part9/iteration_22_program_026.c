#ifdef __cplusplus
#include <iostream>
#endif

/* Global volatile to control conditional compilation */
volatile int parser_test_condition = 0;

int main() {
    /* Valid main function that will compile successfully */
    return 0;
}

/* ===== C and C++ compatible errors ===== */

/* RT_EXTERN: Missing 'extern' in linkage specification */
#if 0
void test_extern_error() {
    /* Missing 'extern' before "C" */
    "C" void linkage_func(void);
}
#endif

/* RT_STATIC_ASSERT: Missing 'static_assert' keyword */
#ifdef __cplusplus
template<bool> struct test_static_assert_error {
    /* Missing 'static_assert' keyword */
    (sizeof(int) == 4, "int must be 4 bytes");
};
#else
#if 0
/* C11 static_assert without keyword */
(1, "assertion failed");
#endif
#endif

/* RT_ASM: Missing 'asm' keyword in inline assembly */
#if 0
void test_asm_error() {
    int x;
    /* Missing 'asm' keyword */
    volatile ("mov %0, %1" : "=r"(x) : "r"(5));
}
#endif

/* ===== C++ only errors (inside __cplusplus guard) ===== */
#ifdef __cplusplus

/* RT_DECLTYPE: Misspelled 'decltype' in trailing return type */
#if 0
auto test_decltype_error() -> etype(5 + 3) {
    return 5 + 3;
}
#endif

/* RT_OPERATOR: Missing 'operator' in overload definition */
class MyClass {
    int value;
public:
    MyClass(int v) : value(v) {}
};

#if 0
/* Missing 'operator' keyword */
int +(MyClass a, MyClass b) {
    return a.value + b.value;
}
#endif

/* RT_CLASS: Missing 'class' in class definition */
#if 0
/* Missing 'class' keyword */
MissingClassKeyword {
    int x;
    void method() {}
};
#endif

/* RT_TEMPLATE: Missing 'template' keyword */
#if 0
/* Missing 'template' keyword */
<typename T>
void template_func(T x) {
    // function body
}
#endif

/* RT_NAMESPACE: Missing 'namespace' keyword */
#if 0
/* Missing 'namespace' keyword */
MyNamespace {
    int namespace_var;
}
#endif

/* RT_USING: Missing 'using' keyword */
#if 0
/* Missing 'using' keyword */
namespace std;
#endif

/* RT_TRY: Missing 'try' keyword */
#if 0
void test_try_error() {
    /* Missing 'try' keyword */
    {
        throw 42;
    }
    catch (...) {
        // handle exception
    }
}
#endif

/* ===== Preprocessor/Attribute interaction tests ===== */

/* Macro that expands to incomplete syntax */
#define BAD_CLASS class
#define BAD_TEMPLATE template

#if 0
/* Using macro that should be followed by identifier */
BAD_ IncompleteType {};
#endif

/* Attribute before syntax error */
#if 0
void __attribute__((always_inline)) test_attr_error() 
    /* Missing 'try' after attribute */
    {
        throw 1;
    } catch(...) {}
#endif

/* ===== Conditional parsing with volatile control flow ===== */

/* This function won't be called, but will be parsed */
void trigger_conditional_errors() {
    volatile int cond = 0;
    
    /* Each error in its own conditional block */
    if (parser_test_condition == 1) {
        /* RT_EXTERN error */
        "C" int conditional_extern_error(void);
    }
    
    if (parser_test_condition == 2) {
        /* RT_CLASS error in C++ mode */
        class {
            int x;
        } instance;
    }
    
    if (parser_test_condition == 3) {
        /* RT_OPERATOR error */
        class Dummy {};
        int +(Dummy a, Dummy b) { return 0; }
    }
}

/* Nested scope errors */
namespace Outer {
#if 0
    /* RT_NAMESPACE error inside namespace */
    Inner {
        int nested_error;
    }
#endif
    
    class Container {
#if 0
        /* RT_TEMPLATE error inside class */
        <typename T>
        void member_template() {}
#endif
    };
}

#endif /* __cplusplus */

/* ===== C-specific variants ===== */
#ifndef __cplusplus

/* C version of static_assert error */
#if 0
_Static_assert (1, "message");  /* Correct */
(1, "missing _Static_assert keyword");  /* Error */
#endif

/* C version of asm error */
#if 0
void c_asm_error(void) {
    int y;
    /* Missing 'asm' keyword */
    ("movl $5, %0" : "=r"(y));
}
#endif

#endif /* !__cplusplus */
