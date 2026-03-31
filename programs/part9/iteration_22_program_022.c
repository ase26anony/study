/* Test program to trigger uncovered error recovery blocks in gcc/parser.cc */
/* Compile with: g++ -xc++ -fsyntax-only -O0 -fdiagnostics-parseable-fixits */

#ifdef __cplusplus
#include <iostream>
#endif

volatile int parser_test_condition = 0;

int main() {
    /* Valid main function that will compile successfully */
    return 0;
}

/* Use #if 0 blocks to isolate each syntax error - parser will still see them */
#if 0
/* RT_EXTERN error in C++ mode */
void test_extern_error() {
    /* Missing 'extern' in linkage specification */
    "C" void missing_extern_func();  /* Error: expected 'extern' */
}
#endif

#if 0
/* RT_STATIC_ASSERT error */
void test_static_assert_error() {
    /* Missing 'static_assert' keyword */
    (1 == 1, "Static assert failed");  /* Error: expected 'static_assert' */
}
#endif

#ifdef __cplusplus
#if 0
/* RT_DECLTYPE error */
auto test_decltype_error() -> etype(parser_test_condition) {
    /* Misspelled decltype in trailing return type */
    return parser_test_condition;  /* Error: expected 'decltype' */
}
#endif

#if 0  
/* RT_OPERATOR error */
struct MyClass {
    int value;
};

/* Missing 'operator' keyword in operator overload */
int +(MyClass a, MyClass b) {  /* Error: expected 'operator' */
    return a.value + b.value;
}
#endif

#if 0
/* RT_CLASS error */
/* Missing 'class' keyword in class definition */
MyClassDefinition {  /* Error: expected 'class' */
    int x;
    int y;
};
#endif

#if 0
/* RT_TEMPLATE error */
/* Missing 'template' keyword */
<typename T>  /* Error: expected 'template' */
void template_func(T t) {}
#endif

#if 0
/* RT_NAMESPACE error */
/* Missing 'namespace' keyword */
MyNamespace {  /* Error: expected 'namespace' */
    int value;
}
#endif

#if 0
/* RT_USING error */
/* Missing 'using' keyword */
namespace std;  /* Error: expected 'using' */
#endif

#if 0
/* RT_TRY error */
void test_try_error() {
    /* Missing 'try' keyword */
    {  /* Error: expected 'try' */
        throw 1;
    } catch (...) {
        /* handle */
    }
}
#endif
#endif /* __cplusplus */

#if 0
/* RT_ASM error in C mode */
void test_asm_error() {
    /* Missing 'asm' keyword for inline assembly */
    volatile ("nop");  /* Error: expected 'asm' */
}
#endif

/* Additional tests with preprocessor interactions */
#if 0
#define BAD_TEMPLATE template
BAD_ <typename T> void bad_func() {}  /* Error in macro expansion */
#endif

#if 0
#define BAD_CLASS class
BAD_ ConfusingType { int x; };  /* Error: expected 'class' */
#endif

/* Tests with __attribute__ interactions */
#if 0
__attribute__((always_inline))
"C" void attr_extern_error();  /* Error after attribute: expected 'extern' */
#endif

#ifdef __cplusplus
#if 0
__attribute__((warn_unused_result))
<typename T> void attr_template_error() {}  /* Error: expected 'template' */
#endif
#endif

/* Tests with volatile control flow - parser sees these but compiler may optimize */
void volatile_controlled_errors() {
    volatile int flag = 0;
    
    if (flag) {
        /* RT_EXTERN error */
        "C" void volatile_extern_error();
    }
    
    if (flag) {
        /* RT_STATIC_ASSERT error */
        (flag == 0, "Volatile assert");
    }
    
#ifdef __cplusplus
    if (flag) {
        /* RT_CLASS error in nested scope */
        struct Inner {
            BadClassDefinition { int x; };  /* Error: expected 'class' */
        };
    }
    
    if (flag) {
        /* RT_TRY error in function scope */
        {
            throw flag;
        } catch (...) {}
    }
#endif
}

/* Test in namespace scope */
#ifdef __cplusplus
namespace TestNamespace {
#if 0
    /* RT_USING error at namespace scope */
    namespace boost;  /* Error: expected 'using' */
#endif

#if 0
    /* RT_TEMPLATE error at namespace scope */
    <typename T>  /* Error: expected 'template' */
    class NamespaceTemplate {};
#endif
}
#endif

/* Final conditional compilation to include one error at a time */
/* Change #if 0 to #if 1 for each block below to test individually */

#if 0
/* RT_EXTERN test */
extern "C" {
    /* Valid code */
    void valid_func() {}
}
"C" void error_func();  /* Error: expected 'extern' */
#endif

#if 0
/* RT_STATIC_ASSERT test */
static_assert(1 == 1, "Valid");
(1 == 2, "Invalid");  /* Error: expected 'static_assert' */
#endif

#ifdef __cplusplus
#if 0
/* RT_DECLTYPE test */
auto valid_func() -> decltype(parser_test_condition) { return 0; }
auto error_func() -> etype(parser_test_condition) { return 0; }  /* Error */
#endif

#if 0
/* RT_OPERATOR test */
struct Num {
    int operator+(Num other) { return 0; }  /* Valid */
    int +(Num other) { return 0; }  /* Error: expected 'operator' */
};
#endif
#endif
