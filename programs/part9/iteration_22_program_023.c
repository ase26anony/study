/* Test program to trigger uncovered error recovery blocks in gcc/parser.cc */
/* Compile with: g++ -xc++ -fsyntax-only -O0 -fdiagnostics-parseable-fixits */

#ifdef __cplusplus
#include <type_traits>
#endif

volatile int condition = 0;

/* RT_EXTERN - missing extern in linkage specification */
void test_extern_error() {
    /* In C++ mode */
    "C" void c_func();  /* Missing 'extern' before linkage spec */
    
    /* In C mode (via conditional) */
#ifdef __cplusplus
    /* Already covered above */
#else
    "C" void c_func_c();  /* Missing 'extern' in C mode */
#endif
}

/* RT_STATIC_ASSERT - missing static_assert keyword */
void test_static_assert_error() {
    /* C++11 static_assert without keyword */
    (1 == 1, "Should be true");  /* Missing 'static_assert' */
    
    /* C11 _Static_assert variant for C mode */
#ifndef __cplusplus
    _Static_assert  /* This is correct, but we'll create error below */
#endif
    
    /* Using macro to hide keyword */
#define STATIC_ASSERT static_assert
    /* Missing token after macro */
    STATIC_ (1, "fail");  /* Deliberate error */
}

#ifdef __cplusplus
/* RT_DECLTYPE - missing decltype in trailing return */
auto test_decltype_error() -> etype(condition) {  /* Missing 'decltype' */
    return condition;
}

/* RT_OPERATOR - missing operator in overload */
struct MyClass {
    int value;
};

/* Missing 'operator' keyword */
int +(MyClass a, MyClass b) {  /* Should be: operator+ */
    return a.value + b.value;
}

/* RT_CLASS - missing class in definition */
/* Missing 'class' keyword */
MyClassType {  /* Should be: class MyClassType */
public:
    int x;
};

/* RT_TEMPLATE - missing template keyword */
/* Missing 'template' */
<typename T>  /* Should be: template<typename T> */
void template_func() {}

/* RT_NAMESPACE - missing namespace keyword */
/* Missing 'namespace' */
my_ns {  /* Should be: namespace my_ns */
    int x = 42;
}

/* RT_USING - missing using keyword */
/* Missing 'using' */
namespace std;  /* Should be: using namespace std; */

/* RT_TRY - missing try keyword */
void test_try_error() {
    /* Missing 'try' */
    {  /* Should be: try { */
        throw 1;
    } catch (...) {
        /* handle */
    }
}
#endif /* __cplusplus */

/* RT_ASM - missing asm keyword */
void test_asm_error() {
    /* Missing 'asm' */
    volatile ("nop");  /* Should be: asm volatile ("nop") */
    
    /* With __asm__ variant for GNU C */
    __asm__  /* Correct, but create error variant */
    
    /* Using attribute to complicate parsing */
    __attribute__((noinline))
    ("nop");  /* Missing 'asm' after attribute */
}

/* Main function with conditional error blocks */
int main() {
    volatile int selector = 0;
    
    /* Use preprocessor to control which errors are parsed */
#if 0
    /* Block 1: RT_EXTERN error */
    test_extern_error();
#endif
    
#if 0
    /* Block 2: RT_STATIC_ASSERT error */
    test_static_assert_error();
#endif
    
#ifdef __cplusplus
#if 0
    /* Block 3: RT_DECLTYPE error (C++ only) */
    auto x = test_decltype_error();
#endif

#if 0
    /* Block 4: RT_OPERATOR error (C++ only) */
    MyClass a{1}, b{2};
    int sum = a + b;
#endif

#if 0
    /* Block 5: RT_CLASS error (C++ only) */
    MyClassType obj;
    obj.x = 5;
#endif

#if 0
    /* Block 6: RT_TEMPLATE error (C++ only) */
    template_func<int>();
#endif

#if 0
    /* Block 7: RT_NAMESPACE error (C++ only) */
    my_ns::x = 10;
#endif

#if 0
    /* Block 8: RT_USING error (C++ only) */
    cout << "test" << endl;
#endif

#if 0
    /* Block 9: RT_TRY error (C++ only) */
    test_try_error();
#endif
#endif /* __cplusplus */

#if 0
    /* Block 10: RT_ASM error */
    test_asm_error();
#endif

    /* Use runtime conditions to include some errors */
    if (selector == 1) {
        /* RT_EXTERN in C context */
#ifndef __cplusplus
        "C" void func();  /* Error */
#endif
    } else if (selector == 2) {
        /* RT_STATIC_ASSERT with macro trick */
#define BAD_STATIC static_assert
        BAD_ (1, "test");  /* Error: BAD_ not defined */
    }
#ifdef __cplusplus
    else if (selector == 3) {
        /* RT_CLASS with attribute interaction */
        __attribute__((packed))
        BadClass {  /* Missing 'class' after attribute */
            int x;
        };
    }
#endif

    return 0;
}

/* Additional test cases in different scopes */
namespace Outer {
#ifdef __cplusplus
    /* RT_TEMPLATE error inside namespace */
    <typename T>  /* Missing 'template' */
    class Inner {};
#endif
    
    void func() {
        /* RT_ASM error inside function in namespace */
        ("nop");  /* Missing 'asm' */
    }
}

/* Test with volatile-controlled dead code */
void volatile_controlled_errors() {
    volatile int flag = 0;
    
    if (flag) {
        /* RT_EXTERN */
        "C" {  /* Missing 'extern' */
            int x;
        }
    }
    
#ifdef __cplusplus
    if (flag) {
        /* RT_OPERATOR inside class method */
        struct Local {
            int operator  /* Incomplete, force error recovery */
        };
    }
#endif
}
