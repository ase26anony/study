/* Test program to trigger specific parser error recovery paths */
#ifdef __cplusplus
#include <iostream>
#endif

volatile int parser_test_condition = 0;

/* RT_EXTERN - missing 'extern' in linkage specification */
void test_extern_error(void) {
    /* In C and C++: linkage specification without 'extern' */
    if (parser_test_condition) {
        "C" void missing_extern_func(void);  /* Error: expected 'extern' */
    }
}

/* RT_STATIC_ASSERT - missing 'static_assert' keyword */
void test_static_assert_error(void) {
    /* In C11/C++11 and later */
    if (parser_test_condition) {
        (1 == 1, "Assertion failed");  /* Error: expected 'static_assert' */
    }
}

#ifdef __cplusplus
/* C++ specific error tests */

/* RT_DECLTYPE - misspelled in trailing return type */
auto test_decltype_error() -> etype(parser_test_condition) {  /* Error: expected 'decltype' */
    return parser_test_condition;
}

/* RT_OPERATOR - missing in operator overload definition */
class MyClass {
    int value;
public:
    MyClass(int v) : value(v) {}
};

int +(MyClass a, MyClass b) {  /* Error: expected 'operator' */
    return a.value + b.value;
}

/* RT_CLASS - missing in class definition */
MyIncompleteClass {  /* Error: expected 'class' (or 'struct'/'union') */
    int x;
    void method();
};

/* RT_TEMPLATE - missing template keyword */
<typename T>  /* Error: expected 'template' */
void template_missing_func(T t) {
    (void)t;
}

/* RT_NAMESPACE - missing namespace keyword */
my_namespace {  /* Error: expected 'namespace' */
    int namespace_var;
}

/* RT_USING - missing using keyword */
namespace std;  /* Error: expected 'using' */

/* RT_TRY - missing try keyword */
void test_try_error() {
    {  /* Error: expected 'try' */
        throw 42;
    } catch (...) {
        /* handler */
    }
}

#endif /* __cplusplus */

/* RT_ASM - missing asm keyword (works in C and C++) */
void test_asm_error(void) {
    if (parser_test_condition) {
        volatile ("nop");  /* Error: expected 'asm' */
    }
}

/* Additional tests with preprocessor interactions */
#define BAD_TEMPLATE template
#define BAD_CLASS class

void test_macro_errors(void) {
#ifdef __cplusplus
    /* Macro expansion that's missing part of the keyword */
    BAD_ MyType {};  /* Error after macro expansion */
    
    /* Attribute followed by error */
    __attribute__((deprecated)) MyClass2 {  /* Error: expected 'class' */
        int y;
    };
#endif
}

/* Main function - valid and compilable */
int main(void) {
    /* Use volatile to prevent dead code elimination */
    volatile int test_case = 0;
    
    /* Test each error case in isolation */
    if (test_case == 1) {
        test_extern_error();
    }
    else if (test_case == 2) {
        test_static_assert_error();
    }
#ifdef __cplusplus
    else if (test_case == 3) {
        /* decltype error - function won't compile */
        auto result = test_decltype_error();
        (void)result;
    }
    else if (test_case == 4) {
        /* operator error */
        MyClass a(1), b(2);
        int sum = +(a, b);  /* This will fail to compile */
        (void)sum;
    }
    else if (test_case == 5) {
        /* class definition error */
        MyIncompleteClass obj;  /* Won't compile */
        (void)obj;
    }
    else if (test_case == 6) {
        /* template error */
        template_missing_func(42);  /* Won't compile */
    }
    else if (test_case == 7) {
        /* namespace error */
        my_namespace::namespace_var = 42;  /* Won't compile */
    }
    else if (test_case == 8) {
        /* using error - already at file scope */
    }
    else if (test_case == 9) {
        test_try_error();
    }
#endif
    else if (test_case == 10) {
        test_asm_error();
    }
    else if (test_case == 11) {
        test_macro_errors();
    }
    
    return 0;
}

/* Additional C-specific tests */
#ifndef __cplusplus
/* In C mode, test static_assert from C11 */
#include <assert.h>
void c_specific_tests(void) {
    /* This should trigger RT_STATIC_ASSERT in C mode */
    _Static_assert(1 == 1, "C11 static assert");
    
    /* Try to trigger error with incomplete static_assert */
    if (parser_test_condition) {
        _Static_(1 == 1, "Missing assert part");  /* Error */
    }
}
#endif
