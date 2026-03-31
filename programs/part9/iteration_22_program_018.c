/* Test program to trigger specific parser error recovery paths in GCC */
#ifdef __cplusplus
#include <iostream>
#endif

/* Volatile control variable to prevent dead code elimination */
volatile int parser_test_condition = 0;

int main() {
    /* Valid main function that compiles successfully */
    return 0;
}

/* ==================== C and C++ compatible errors ==================== */

/* RT_EXTERN: Missing 'extern' in linkage specification */
#if 0
void test_extern_error() {
    /* Missing 'extern' before "C" */
    "C" void linkage_func(void);
}
#endif

/* RT_STATIC_ASSERT: Missing 'static_assert' keyword */
#if 0
void test_static_assert_error() {
    /* Missing 'static_assert' keyword */
    (1 == 1, "Assertion failed");
}
#endif

/* RT_ASM: Missing 'asm' keyword for inline assembly */
#if 0
void test_asm_error() {
    int x;
    /* Missing 'asm' keyword */
    volatile ("mov %0, %1" : "=r"(x) : "r"(5));
}
#endif

/* ==================== C++ only errors (inside #ifdef) ==================== */
#ifdef __cplusplus

/* RT_DECLTYPE: Missing 'decltype' in trailing return type */
#if 0
auto test_decltype_error() -> /* Missing 'decltype' */ (x + y) {
    return 0;
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
    int value = 42;
}
#endif

/* RT_USING: Missing 'using' keyword */
#if 0
/* Missing 'using' keyword */
namespace std;

void test_using() {
    cout << "test" << endl;
}
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

/* ==================== Additional tests with preprocessor ==================== */

/* Test with macro expansion */
#define EXPECT_CLASS class
#if 0
/* Macro that expands to incomplete token */
EXPECT_ MyIncompleteType {
    int member;
};
#endif

/* Test with attributes */
#if 0
void __attribute__((deprecated)) test_attr_extern_error() {
    /* Missing 'extern' after attribute */
    "C++" void another_func();
}
#endif

/* Nested scope test for RT_CLASS */
#if 0
namespace Outer {
    /* Missing 'class' inside namespace */
    InnerType {
        int data;
    };
}
#endif

/* Template with missing 'template' in different context */
#if 0
template<typename U>
class Container {
    /* Missing 'template' for dependent type */
    <typename V>
    void nested_template() {}
};
#endif

/* ==================== Conditional compilation with volatile ==================== */

/* Use volatile to control which error path is parsed */
void conditional_errors() {
    if (parser_test_condition == 1) {
        /* RT_EXTERN error in conditional context */
        "C" void conditional_linkage(void);
    }
    else if (parser_test_condition == 2) {
        /* RT_ASM error in conditional context */
        volatile ("nop");
    }
#ifdef __cplusplus
    else if (parser_test_condition == 3) {
        /* RT_OPERATOR error */
        class TempClass {
            int val;
        };
        int *(TempClass a, TempClass b);
    }
    else if (parser_test_condition == 4) {
        /* RT_TRY error in function scope */
        {
            throw 1;
        }
        catch (...) {}
    }
#endif
}

#endif /* __cplusplus */

/* ==================== C-specific variants ==================== */
#ifndef __cplusplus

/* C version of static_assert error (C11) */
#if 0
_Static_assert(1 == 1, "Message");
/* Trigger error by using wrong syntax */
_Assert(1 == 2, "Fail");
#endif

/* C version with missing 'asm' */
#if 0
void c_asm_test(void) {
    int result;
    /* Missing 'asm' */
    ("movl $5, %0" : "=r"(result));
}
#endif

#endif /* !__cplusplus */
