#ifdef __cplusplus
#include <iostream>
#endif

/* Volatile control variable to ensure all code paths are parsed */
volatile int parse_condition = 0;

int main() {
    /* Valid main function that will compile successfully */
    return 0;
}

/* ====== C/C++ COMMON TOKENS ====== */

/* RT_EXTERN - Missing 'extern' in linkage specification */
#if 0
void test_extern_error() {
    /* Missing 'extern' before linkage specifier */
    "C" void linkage_func(void);
    
    /* Another variant with attributes */
    __attribute__((weak)) "C" int weak_symbol;
}
#endif

/* RT_STATIC_ASSERT - Missing 'static_assert' keyword */
#ifdef __cplusplus
namespace static_assert_test {
    /* Missing static_assert keyword */
    template<bool B>
    struct test {
        /* Error: missing static_assert */
        (B, "Template parameter must be true");
    };
}
#else
/* C version */
#if 0
/* Missing static_assert in C */
(1 == 1, "Assertion failed");
#endif
#endif

/* RT_ASM - Missing 'asm' keyword for inline assembly */
#if 0
void test_asm_error() {
    int result;
    /* Missing 'asm' keyword */
    volatile (
        "mov %1, %0\n\t"
        : "=r"(result)
        : "r"(42)
    );
    
    /* With goto */
    __asm__ goto volatile ("jmp %l0" : : : : label);
    label:
    return;
}
#endif

/* ====== C++ ONLY TOKENS ====== */
#ifdef __cplusplus

/* RT_DECLTYPE - Missing 'decltype' in trailing return type */
#if 0
auto test_decltype_error() -> etype(5 + 3) {
    return 5 + 3;
}

/* Another variant with template */
template<typename T>
auto template_decltype_error(T x) -> etype(x + 1) {
    return x + 1;
}
#endif

/* RT_OPERATOR - Missing 'operator' in overload definition */
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

/* Conversion operator missing 'operator' */
class Convertible {
    int +(void) { return 42; }  /* Should be: operator int() */
};
#endif

/* RT_CLASS - Missing 'class' keyword in definition */
#if 0
/* Missing 'class' before class name */
MyClassDefinition {
    int x;
    void method();
};

/* Template class missing 'class' */
template<typename T>
TemplateDef {
    T data;
};
#endif

/* RT_TEMPLATE - Missing 'template' keyword */
#if 0
/* Missing 'template' before template parameters */
<typename T, typename U>
void template_func(T a, U b) {
    return;
}

/* Class template missing 'template' */
<class T>
class MissingTemplateKeyword {
    T data;
};
#endif

/* RT_NAMESPACE - Missing 'namespace' keyword */
#if 0
/* Missing 'namespace' before namespace name */
MyNamespace {
    int value;
    void function();
}

/* Inline namespace missing keyword */
inline MyInlineNS {
    int x;
}
#endif

/* RT_USING - Missing 'using' keyword */
#if 0
/* Missing 'using' for namespace */
namespace std;

/* Missing 'using' for alias */
typename T = std::vector<int>;

/* Missing 'using' in using declaration */
std::cout;
#endif

/* RT_TRY - Missing 'try' keyword */
#if 0
void test_try_error() {
    /* Missing 'try' before block */
    {
        throw std::runtime_error("error");
    }
    catch (const std::exception& e) {
        /* Handle exception */
    }
    
    /* Function try block missing 'try' */
    MyClass::MyClass(int x) 
        : value(x)
    {
        /* constructor body */
    }
    catch (...) {
        /* handler */
    }
}
#endif

/* ====== MACRO AND ATTRIBUTE INTERACTIONS ====== */

/* RT_CLASS with macro expansion */
#if 0
#define CLASS_DEF class
#define BAD_CLASS_DEF CLASS

/* Macro expands to missing 'class' */
BAD_ IncompleteType {
    int x;
};
#endif

/* RT_EXTERN with attributes */
#if 0
__attribute__((visibility("default")))
"C++"
void attributed_func(void) {
    /* Function body */
}
#endif

/* RT_TEMPLATE in nested context */
#if 0
namespace outer {
    template<typename T>
    class OuterClass {
        /* Missing 'template' in member template */
        <typename U>
        void member_template(U u) {
            T t;
            t = u;
        }
    };
}
#endif

/* ====== VOLATILE-CONTROLLED ERROR BLOCKS ====== */

/* Use volatile condition to control parsing of error blocks */
void parse_error_blocks() {
    /* Each block is conditionally compiled based on volatile variable */
    /* This ensures parser sees them but compiler may optimize away */
    
    if (parse_condition == 1) {
        /* RT_EXTERN error in C context */
        #ifndef __cplusplus
        "C" int c_func(void);
        #endif
    }
    
    if (parse_condition == 2) {
        /* RT_STATIC_ASSERT error */
        #ifdef __cplusplus
        (sizeof(int) == 4, "int must be 4 bytes");
        #endif
    }
    
    if (parse_condition == 3) {
        /* RT_ASM error */
        volatile ("nop");
    }
    
    #ifdef __cplusplus
    if (parse_condition == 4) {
        /* RT_DECLTYPE error */
        auto f() -> etype(42);
    }
    
    if (parse_condition == 5) {
        /* RT_OPERATOR error */
        class OpClass {};
        int +(OpClass a, OpClass b);
    }
    
    if (parse_condition == 6) {
        /* RT_CLASS error */
        TestClass { int x; };
    }
    
    if (parse_condition == 7) {
        /* RT_TEMPLATE error */
        <typename T> void tmpl_func(T t) {}
    }
    
    if (parse_condition == 8) {
        /* RT_NAMESPACE error */
        ErrorNS { int x; };
    }
    
    if (parse_condition == 9) {
        /* RT_USING error */
        namespace std;
    }
    
    if (parse_condition == 10) {
        /* RT_TRY error */
        { throw 1; } catch (...) {}
    }
    #endif
}

/* Nested error in class scope */
#ifdef __cplusplus
class Container {
    #if 0
    /* RT_OPERATOR error inside class */
    int +(int x) { return x; }
    #endif
    
    #if 0  
    /* RT_TEMPLATE error inside class */
    <typename U>
    U convert() { return U(); }
    #endif
};
#endif

/* Error in template context */
#ifdef __cplusplus
template<typename T>
void template_error_test() {
    #if 0
    /* RT_DECLTYPE in template */
    auto get_type() -> etype(T());
    #endif
    
    #if 0
    /* RT_STATIC_ASSERT in template */
    (sizeof(T) > 0, "Type must have size");
    #endif
}
#endif
