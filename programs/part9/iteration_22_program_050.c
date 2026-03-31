#ifdef __cplusplus
#include <iostream>
#endif

/* Volatile control variable to prevent dead code elimination */
volatile int parser_mode = 0;

int main() {
    /* Valid main function that compiles successfully */
    return 0;
}

/* ==================== C/C++ COMMON ERRORS ==================== */

/* RT_EXTERN error - missing 'extern' in linkage specification */
#if 0
void test_extern_error() {
    /* Missing 'extern' before "C" */
    "C" void foreign_func(void);  /* Should trigger RT_EXTERN error */
}
#endif

/* RT_STATIC_ASSERT error - missing 'static_assert' keyword */
#if 0
void test_static_assert_error() {
    /* Missing 'static_assert' keyword */
    (1 == 1, "Assertion failed");  /* Should trigger RT_STATIC_ASSERT error */
}
#endif

/* RT_ASM error - missing 'asm' keyword for inline assembly */
#if 0
void test_asm_error() {
    int result;
    /* Missing 'asm' keyword */
    volatile ("mov %0, 1" : "=r"(result));  /* Should trigger RT_ASM error */
}
#endif

#ifdef __cplusplus
/* ==================== C++ ONLY ERRORS ==================== */

/* RT_DECLTYPE error - misspelled 'decltype' in trailing return */
#if 0
auto test_decltype_error() -> etype(5 + 3) {  /* Should trigger RT_DECLTYPE error */
    return 5 + 3;
}
#endif

/* RT_OPERATOR error - missing 'operator' in overload definition */
#if 0
class MyClass {
    int value;
public:
    MyClass(int v) : value(v) {}
};

/* Missing 'operator' keyword */
int +(MyClass a, MyClass b) {  /* Should trigger RT_OPERATOR error */
    return a.value + b.value;
}
#endif

/* RT_CLASS error - missing 'class' in class definition */
#if 0
/* Missing 'class' keyword */
MyClass2 {  /* Should trigger RT_CLASS error */
public:
    int x;
private:
    int y;
};
#endif

/* RT_TEMPLATE error - missing 'template' keyword */
#if 0
/* Missing 'template' keyword */
<typename T>  /* Should trigger RT_TEMPLATE error */
void template_func(T value) {
    // function body
}
#endif

/* RT_NAMESPACE error - missing 'namespace' keyword */
#if 0
/* Missing 'namespace' keyword */
my_namespace {  /* Should trigger RT_NAMESPACE error */
    int global_var;
    
    void namespace_func() {
        // function in namespace
    }
}
#endif

/* RT_USING error - missing 'using' keyword */
#if 0
/* Missing 'using' keyword */
namespace std;  /* Should trigger RT_USING error */
#endif

/* RT_TRY error - missing 'try' keyword */
#if 0
void test_try_error() {
    /* Missing 'try' keyword */
    {  /* Should trigger RT_TRY error */
        throw 42;
    } catch (int e) {
        // handle exception
    }
}
#endif

/* ==================== PREPROCESSOR INTERACTION ERRORS ==================== */

/* Error inside macro expansion */
#if 0
#define BAD_CLASS class
BAD_ MyMacroClass {  /* Should trigger RT_CLASS error from macro */
    int member;
};
#endif

/* Error after attribute */
#if 0
void __attribute__((always_inline)) test_attr_error() 
    /* Missing 'try' after attribute */
    {  /* Should trigger RT_TRY error */
        throw "error";
    } catch (const char* e) {
    }
#endif

/* ==================== NESTED SCOPE ERRORS ==================== */

/* Error inside namespace */
#if 0
namespace Outer {
    /* Missing 'namespace' for inner namespace */
    Inner {  /* Should trigger RT_NAMESPACE error */
        int nested_var;
    };
}
#endif

/* Error inside class */
#if 0
class Container {
public:
    /* Missing 'template' inside class */
    <typename U>  /* Should trigger RT_TEMPLATE error */
    void method() {}
};
#endif

/* ==================== VOLATILE-CONTROLLED ERRORS ==================== */

/* These will be parsed but not compiled due to volatile condition */
void volatile_controlled_errors() {
    volatile int trigger = 0;
    
    if (trigger) {
        /* RT_EXTERN error in volatile block */
        "C++" void volatile_extern_func();  /* Missing 'extern' */
    }
    
    if (trigger) {
        /* RT_STATIC_ASSERT error in volatile block */
        (sizeof(int) == 4, "Size mismatch");  /* Missing 'static_assert' */
    }
    
    if (trigger) {
        /* RT_ASM error in volatile block */
        volatile ("nop");  /* Missing 'asm' */
    }
    
    if (trigger) {
        /* RT_DECLTYPE error in volatile block */
        auto volatile_func() -> decltyp(5);  /* Misspelled 'decltype' */
    }
    
    if (trigger) {
        /* RT_OPERATOR error in volatile block */
        bool ==(int a, int b) { return a == b; }  /* Missing 'operator' */
    }
}
#endif /* __cplusplus */

/* ==================== C MODE SPECIFIC ERRORS ==================== */
#ifndef __cplusplus
/* C-specific static_assert error (C11) */
#if 0
_Static_assert missing_error = (1 == 1, "C static assert");  /* Wrong syntax */
#endif

/* C-specific asm error */
#if 0
void c_asm_error() {
    int x;
    /* Missing 'asm' in C */
    volatile ("movl $1, %0" : "=r"(x));  /* Should trigger RT_ASM error */
}
#endif

/* C-specific extern error with attributes */
#if 0
void __attribute__((weak)) c_extern_error() {
    /* Missing 'extern' with attribute */
    "C" __attribute__((noreturn)) void c_func(void);  /* Should trigger RT_EXTERN error */
}
#endif
#endif /* !__cplusplus */
