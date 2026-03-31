#ifdef __cplusplus
#include <iostream>
#endif

/* Volatile variable to control flow without optimization removing code */
volatile int parser_test_condition = 0;

int main() {
    /* Valid main function that compiles successfully */
    return 0;
}

/* ========== ERROR BLOCKS ========== */
/* Each block contains a deliberate syntax error targeting a specific token */

/* RT_EXTERN - C and C++ */
#ifdef __cplusplus
extern "C" {
#endif

void test_extern_error() {
    /* Missing 'extern' in linkage specification */
    if (parser_test_condition) {
        "C" void missing_extern_func();  /* Error: expected 'extern' */
    }
}

#ifdef __cplusplus
}
#endif

/* RT_STATIC_ASSERT - C11 and C++ */
void test_static_assert_error() {
    /* Missing 'static_assert' keyword */
    if (parser_test_condition) {
        (1 == 1, "Assertion failed");  /* Error: expected 'static_assert' */
    }
    
#ifdef __cplusplus
    /* C++17 static_assert without message */
    if (parser_test_condition) {
        (1 == 1);  /* Error: expected 'static_assert' */
    }
#endif
}

#ifdef __cplusplus
/* C++-specific errors */

/* RT_DECLTYPE */
auto test_decltype_error() -> /* Missing 'decltype' */ etype(parser_test_condition) {
    /* Error: expected 'decltype' in trailing return type */
    return parser_test_condition;
}

/* RT_OPERATOR */
class MyClass {
    int value;
public:
    MyClass(int v) : value(v) {}
};

/* Missing 'operator' in operator overload */
int /* operator */ +(MyClass a, MyClass b) {  /* Error: expected 'operator' */
    return a.value + b.value;
}

/* RT_CLASS */
/* Missing 'class' keyword in class definition */
struct /* class */ MissingClassKeyword {  /* Error: expected 'class' */
    int x;
    int y;
};

/* RT_TEMPLATE */
/* Missing 'template' keyword */
<typename T>  /* Error: expected 'template' */
void template_missing() {
    T value = T();
}

/* RT_NAMESPACE */
/* Missing 'namespace' keyword */
/* namespace */ missing_namespace_keyword {  /* Error: expected 'namespace' */
    int x;
}

/* RT_USING */
/* Missing 'using' keyword in using directive */
/* using */ namespace std;  /* Error: expected 'using' */

/* RT_TRY */
void test_try_error() {
    /* Missing 'try' keyword */
    /* try */ {  /* Error: expected 'try' */
        throw 42;
    } catch (...) {
        /* Handle exception */
    }
}

#endif /* __cplusplus */

/* RT_ASM - C and C++ */
void test_asm_error() {
    /* Missing 'asm' keyword in inline assembly */
    if (parser_test_condition) {
        volatile /* asm */ ("nop");  /* Error: expected 'asm' */
    }
}

/* ========== MACRO-BASED ERRORS ========== */
/* Testing parser state machine during macro expansion */

#define EXPECT_CLASS class
#define EXPECT_TEMPLATE template
#define EXPECT_OPERATOR operator

#ifdef __cplusplus
/* Macro expansion missing token */
void macro_errors() {
    /* RT_CLASS via macro */
    if (parser_test_condition) {
        EXPECT_ MyClass {};  /* Error after macro expansion */
    }
    
    /* RT_TEMPLATE via macro */
    if (parser_test_condition) {
        EXPECT_ <typename T> void f() {}  /* Error after macro expansion */
    }
    
    /* RT_OPERATOR via macro */
    if (parser_test_condition) {
        int EXPECT_ +(int a, int b) { return a + b; }  /* Error after macro expansion */
    }
}
#endif

/* ========== ATTRIBUTE INTERACTIONS ========== */
/* Testing error recovery after attribute specifiers */

void __attribute__((always_inline)) test_attribute_error() {
#ifdef __cplusplus
    /* RT_TRY after attribute */
    if (parser_test_condition) {
        __attribute__((unused)) /* try */ {  /* Error: expected 'try' */
            /* empty */
        } catch (...) {}
    }
#endif
}

/* ========== NESTED SCOPE ERRORS ========== */
/* Testing errors in different syntactic contexts */

#ifdef __cplusplus
namespace OuterNamespace {
    /* RT_NAMESPACE in namespace scope */
    /* namespace */ InnerMissing {  /* Error: expected 'namespace' */
        int x;
    }
    
    class Container {
    public:
        /* RT_OPERATOR in class scope */
        int /* operator */()(int x) {  /* Error: expected 'operator' */
            return x * 2;
        }
        
        /* RT_TEMPLATE in class scope */
        /* template */ <typename U>  /* Error: expected 'template' */
        void member_template() {}
    };
}
#endif

/* ========== CONDITIONAL COMPILATION STRUCTURE ========== */
/* Using preprocessor to isolate errors */

#if 0
/* This section is never compiled but shows alternative structure */

/* RT_EXTERN in C mode */
#ifndef __cplusplus
"C" void c_extern_error();  /* Error: expected 'extern' */
#endif

/* RT_STATIC_ASSERT in C11 mode */
#if __STDC_VERSION__ >= 201112L
(1, "C static_assert error");  /* Error: expected 'static_assert' */
#endif

#endif /* 0 */
