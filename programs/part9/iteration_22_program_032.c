#ifdef __cplusplus
#include <iostream>
#endif

/* Volatile control variable to ensure all code paths are parsed */
volatile int parse_control = 0;

int main() {
    /* Valid main function that will compile successfully */
    return 0;
}

/* ===== C/C++ COMMON TOKENS ===== */

/* RT_EXTERN: Missing 'extern' in linkage specification */
#if 0
void test_extern_error() {
    /* Missing 'extern' before "C" */
    "C" void foreign_func(void);  /* Should expect 'extern' */
}
#endif

/* RT_STATIC_ASSERT: Missing 'static_assert' keyword */
#if 0
void test_static_assert_error() {
    /* Missing 'static_assert' keyword */
    (1 == 1, "Assertion failed");  /* Should expect 'static_assert' */
    
    /* Alternative with attribute interaction */
    __attribute__((unused)) (1, "test");
}
#endif

/* RT_ASM: Missing 'asm' keyword for inline assembly */
#if 0
void test_asm_error() {
    int result;
    /* Missing 'asm' keyword */
    volatile (
        "movl $1, %%eax"
        : "=a"(result)
    );  /* Should expect 'asm' */
}
#endif

/* ===== C++ ONLY TOKENS ===== */
#ifdef __cplusplus

/* RT_DECLTYPE: Missing 'decltype' in trailing return type */
#if 0
auto test_decltype_error() -> /* Missing 'decltype' */ (x) {
    return 1;
}

/* Alternative with macro */
#define BAD_DECLTYPE etype
auto func() -> BAD_DECLTYPE(1+2) { return 3; }
#endif

/* RT_OPERATOR: Missing 'operator' in overload definition */
#if 0
class MyClass {
    int value;
public:
    MyClass(int v) : value(v) {}
};

/* Missing 'operator' keyword */
int +(MyClass a, MyClass b) {  /* Should expect 'operator' */
    return a.value + b.value;
}

/* Inside template */
template<typename T>
bool ==(T a, T b) { return true; }  /* Missing 'operator' */
#endif

/* RT_CLASS: Missing 'class' in class definition */
#if 0
/* Missing 'class' keyword */
MyClassType {  /* Should expect 'class' (or 'struct') */
    int member;
};

/* With macro expansion */
#define BAD_CLASS_DEF class
BAD_ MyType {};  /* Missing 'class' after macro */

/* In template context */
template<typename T>
/* Missing 'class' */ Container {
    T data;
};
#endif

/* RT_TEMPLATE: Missing 'template' keyword */
#if 0
/* Missing 'template' keyword */
<typename T>  /* Should expect 'template' */
void template_func(T t) {}

/* Nested in class */
class Outer {
    /* Missing 'template' */
    <typename U>
    class Inner {};
};
#endif

/* RT_NAMESPACE: Missing 'namespace' keyword */
#if 0
/* Missing 'namespace' keyword */
my_namespace {  /* Should expect 'namespace' */
    int value;
}

/* With attributes */
__attribute__((visibility("default"))) 
lib_namespace {  /* Missing 'namespace' */
    void api_func();
}
#endif

/* RT_USING: Missing 'using' keyword */
#if 0
/* Missing 'using' keyword */
namespace std;  /* Should expect 'using' for using directive */

/* In class context */
class TestClass {
    /* Missing 'using' */
    BaseClass::some_type;  /* Should expect 'using' for using declaration */
};
#endif

/* RT_TRY: Missing 'try' keyword */
#if 0
void test_try_error() {
    /* Missing 'try' keyword */
    {  /* Should expect 'try' */
        throw 42;
    } catch (int e) {
        // handle
    }
    
    /* Function try block */
    TestClass() 
    {  /* Missing 'try' for function try block */
        throw 1;
    } catch (...) {
    }
}
#endif

/* ===== CONDITIONAL PARSING BLOCKS ===== */

/* This function contains all errors conditionally based on volatile control */
void trigger_all_errors() {
    /* Each error in its own conditional block */
    if (parse_control == 1) {
        /* RT_EXTERN error */
        "C" void func1(void);
    }
    
    if (parse_control == 2) {
        /* RT_STATIC_ASSERT error */
        (1, "test");
    }
    
    if (parse_control == 3) {
        /* RT_ASM error */
        volatile ("nop");
    }
    
#ifdef __cplusplus
    if (parse_control == 4) {
        /* RT_DECLTYPE error */
        auto f() -> etype(1+2);
    }
    
    if (parse_control == 5) {
        /* RT_OPERATOR error */
        class X {};
        int +(X a, X b);
    }
    
    if (parse_control == 6) {
        /* RT_CLASS error */
        MyTestClass { int x; };
    }
    
    if (parse_control == 7) {
        /* RT_TEMPLATE error */
        <typename T> void f(T) {}
    }
    
    if (parse_control == 8) {
        /* RT_NAMESPACE error */
        TestNS { int y; };
    }
    
    if (parse_control == 9) {
        /* RT_USING error */
        namespace std;
    }
    
    if (parse_control == 10) {
        /* RT_TRY error */
        { throw 1; } catch (...) {}
    }
#endif
}

#endif /* __cplusplus */

/* Additional C-specific tests */
#ifndef __cplusplus
/* RT_EXTERN in C with attribute */
#if 0
__attribute__((weak)) "C" void c_func(void);
#endif

/* RT_STATIC_ASSERT in C11 */
#if 0
_Static_assert  /* This is correct, but we can create error after it */
(1 == 2, "test")  /* Missing ; or other error to trigger recovery */
#endif

/* RT_ASM in C with goto labels */
#if 0
void c_asm_error() {
    int x;
    volatile ("mov %1, %0" : "=r"(x) : "r"(1));
}
#endif
#endif
