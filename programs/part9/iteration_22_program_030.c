/* Test program to trigger specific parser error recovery paths */
#ifdef __cplusplus
#include <iostream>
#endif

/* Volatile control variable to ensure code is parsed but not eliminated */
static volatile int parser_test_condition = 0;

/* Macro-based error triggers */
#define EXPECT_EXTERN_ERROR 1
#define EXPECT_STATIC_ASSERT_ERROR 2
#define EXPECT_DECLTYPE_ERROR 3
#define EXPECT_OPERATOR_ERROR 4
#define EXPECT_CLASS_ERROR 5
#define EXPECT_TEMPLATE_ERROR 6
#define EXPECT_NAMESPACE_ERROR 7
#define EXPECT_USING_ERROR 8
#define EXPECT_ASM_ERROR 9
#define EXPECT_TRY_ERROR 10

int main(void) {
    /* Main function is valid and compilable */
    
    /* Use volatile to prevent dead code elimination */
    volatile int test_case = parser_test_condition;
    
    /* RT_EXTERN - Missing 'extern' in linkage specification */
    if (test_case == EXPECT_EXTERN_ERROR) {
        /* Missing 'extern' before "C" */
        "C" void missing_extern_func(void);
    }
    
    /* RT_STATIC_ASSERT - Missing 'static_assert' keyword */
    if (test_case == EXPECT_STATIC_ASSERT_ERROR) {
        /* Missing 'static_assert' keyword */
        (1 == 1, "static_assert without keyword");
    }
    
#ifdef __cplusplus
    /* C++ specific error cases */
    
    /* RT_DECLTYPE - Missing 'decltype' in trailing return type */
    if (test_case == EXPECT_DECLTYPE_ERROR) {
        struct TestStruct { int x; };
        TestStruct ts;
        /* Missing 'decltype' keyword */
        auto missing_decltype_func() -> (ts.x);
    }
    
    /* RT_OPERATOR - Missing 'operator' in operator overload */
    if (test_case == EXPECT_OPERATOR_ERROR) {
        class MyClass {
            int value;
        public:
            MyClass(int v) : value(v) {}
        };
        /* Missing 'operator' keyword */
        MyClass +(MyClass a, MyClass b) {
            return MyClass(a.value + b.value);
        }
    }
    
    /* RT_CLASS - Missing 'class' in class definition */
    if (test_case == EXPECT_CLASS_ERROR) {
        /* Missing 'class' keyword */
        MissingClassKeyword {
        public:
            int x;
        };
    }
    
    /* RT_TEMPLATE - Missing 'template' keyword */
    if (test_case == EXPECT_TEMPLATE_ERROR) {
        /* Missing 'template' keyword */
        <typename T>
        void missing_template_func(T x) {}
    }
    
    /* RT_NAMESPACE - Missing 'namespace' keyword */
    if (test_case == EXPECT_NAMESPACE_ERROR) {
        /* Missing 'namespace' keyword */
        MissingNamespaceKeyword {
            int x;
        }
    }
    
    /* RT_USING - Missing 'using' keyword */
    if (test_case == EXPECT_USING_ERROR) {
        /* Missing 'using' keyword */
        namespace std;
    }
    
    /* RT_TRY - Missing 'try' keyword */
    if (test_case == EXPECT_TRY_ERROR) {
        /* Missing 'try' keyword */
        {
            throw 42;
        }
        catch (...) {
        }
    }
#endif
    
    /* RT_ASM - Missing 'asm' keyword (works in both C and C++) */
    if (test_case == EXPECT_ASM_ERROR) {
        /* Missing 'asm' keyword */
        volatile ("nop");
    }
    
    /* Additional C-specific variants */
#ifndef __cplusplus
    /* C version of static_assert error */
    if (test_case == EXPECT_STATIC_ASSERT_ERROR) {
        _Static_assert missing_keyword(1 == 1, "Error");
    }
    
    /* C version of asm error with attribute */
    if (test_case == EXPECT_ASM_ERROR + 1) {
        __attribute__((naked))
        volatile ("ret");
    }
#endif
    
    /* Test with macro expansions to trigger parser state issues */
#ifdef __cplusplus
    if (test_case == EXPECT_CLASS_ERROR + 1) {
        #define BAD_CLASS class
        BAD_ MisparsedClass {
            int y;
        };
    }
    
    if (test_case == EXPECT_TEMPLATE_ERROR + 1) {
        #define TEMPLATE_START template
        TEMPLATE_ <class T>
        void another_bad_func(T) {}
    }
#endif
    
    /* Nested scope tests */
#ifdef __cplusplus
    namespace Outer {
        if (test_case == EXPECT_NAMESPACE_ERROR + 1) {
            /* Missing 'namespace' inside another namespace */
            Inner {
                int z;
            }
        }
    }
    
    class Container {
    public:
        if (test_case == EXPECT_OPERATOR_ERROR + 1) {
            /* Missing 'operator' inside class */
            int +(const Container& other) {
                return 0;
            }
        }
    };
#endif
    
    return 0;
}

/* Additional error cases in different contexts */
#ifdef __cplusplus
/* RT_EXTERN in C++ context */
extern "C" {
    /* Missing 'extern' inside linkage spec */
    if (parser_test_condition == EXPECT_EXTERN_ERROR + 1) {
        "C++" void nested_missing_extern(void);
    }
}

/* RT_STATIC_ASSERT at namespace scope */
static_assert_missing(1 == 1, "global scope error");

/* RT_DECLTYPE in template */
template<typename T>
auto template_missing_decltype(T x) -> (x.size());

/* RT_OPERATOR as friend */
class FriendTest {
    friend MyClass +(MyClass, MyClass);
};

/* RT_CLASS with attributes */
__attribute__((packed))
MissingAttrClass {
    char c;
};

/* RT_TEMPLATE in class */
class HasTemplate {
    /* Missing 'template' */
    <typename U>
    static void member_template();
};

/* RT_NAMESPACE with inline */
inline MissingInlineNamespace {
    int w;
};

/* RT_USING with template */
template<typename T>
typename_missing std::vector<T>;

/* RT_ASM with goto label */
void asm_with_label(void) {
    volatile int x;
    asm_label:
    volatile ("mov %0, %1" : "=r"(x));
}

/* RT_TRY in function */
void try_test(void) {
    /* Missing 'try' */
    {
        throw std::runtime_error("test");
    }
    catch (...) {
    }
}
#endif

/* C-specific global scope errors */
#ifndef __cplusplus
/* RT_EXTERN at file scope */
"C" int missing_extern_global = 0;

/* RT_STATIC_ASSERT in struct */
struct CStruct {
    int field;
    static_assert_missing(sizeof(int) == 4, "struct member error");
};

/* RT_ASM in function with attribute */
__attribute__((noinline))
void asm_func(void) {
    volatile ("nop");
}
#endif
