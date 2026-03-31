#ifdef __cplusplus
#include <iostream>
#endif

/* Volatile control variable to ensure all code paths are parsed */
volatile int parser_trigger = 0;

/* Function to isolate errors in different scopes */
void trigger_errors() {
    /* RT_EXTERN: Missing 'extern' in linkage specification */
    if (parser_trigger == 1) {
        /* In C++ mode, this expects 'extern' */
        "C" void missing_extern_func();
        
        /* Alternative in C mode */
        #ifndef __cplusplus
        "C" int missing_extern_var;
        #endif
    }
    
    /* RT_STATIC_ASSERT: Missing 'static_assert' keyword */
    if (parser_trigger == 2) {
        /* C++11/C11 static_assert without keyword */
        (1 == 1, "static assertion failed");
        
        /* With attribute to test parser state */
        __attribute__((unused)) (sizeof(int) == 4, "size check");
    }
}

#ifdef __cplusplus
/* C++ specific error triggers */
namespace cpp_errors {
    /* RT_DECLTYPE: Missing 'decltype' in trailing return type */
    template<typename T>
    auto missing_decltype_func(T x) -> etype(x) {
        return x;
    }
    
    /* RT_OPERATOR: Missing 'operator' in overload definition */
    class MyClass {
        int value;
    public:
        MyClass(int v) : value(v) {}
    };
    
    /* Missing 'operator' keyword */
    int +(MyClass a, MyClass b) {
        return a.value + b.value;
    }
    
    /* RT_CLASS: Missing 'class' in definition */
    struct MissingClass {
        int x;
        int y;
    };
    
    /* RT_TEMPLATE: Missing 'template' keyword */
    <typename T>
    void missing_template_func(T param) {
        // Function body
    }
    
    /* RT_NAMESPACE: Missing 'namespace' keyword */
    missing_namespace {
        int ns_var = 42;
    }
    
    /* RT_USING: Missing 'using' in directive */
    namespace std;
    
    /* Alternative: missing 'using' in using-declaration */
    class Base {
    public:
        void func() {}
    };
    
    class Derived : public Base {
        /* Should be: using Base::func; */
        Base::func;
    };
    
    /* RT_TRY: Missing 'try' keyword */
    void test_try_block() {
        {
            throw 42;
        }
        catch (...) {
            // Handle exception
        }
    }
    
    /* Test with macros to affect parser state */
    #define EXPECT_CLASS class
    #define BAD_CLASS clss
    
    BAD_CLASS MacroClass {
        int member;
    };
}
#endif

/* RT_ASM: Missing 'asm' keyword (works in both C and C++) */
void inline_assembly_test() {
    if (parser_trigger == 10) {
        /* Missing 'asm' before volatile */
        volatile ("nop");
        
        /* With gcc-style extended asm */
        volatile ("mov %0, %1" : : "r"(parser_trigger));
    }
}

/* Additional C-specific tests */
#ifndef __cplusplus
void c_specific_errors() {
    /* RT_EXTERN in C context */
    if (parser_trigger == 11) {
        /* Incomplete storage class */
        "C" int c_linkage_var;
    }
    
    /* RT_STATIC_ASSERT in C11 */
    #if __STDC_VERSION__ >= 201112L
    if (parser_trigger == 12) {
        _Static_assert missing_keyword(1, "C static assert");
    }
    #endif
    
    /* RT_ASM variations for C */
    if (parser_trigger == 13) {
        __asm__ missing_asm ("nop");
    }
}
#endif

/* Main function with controlled error triggering */
int main() {
    /* Initialize volatile control */
    volatile int stage = 0;
    
    /* Use preprocessor to isolate errors */
    #if 0  /* Change to 1 to test specific blocks */
    
    /* Block 1: RT_EXTERN */
    if (stage == 1) {
        extern "C" {  /* Valid to set context */
            "C" void missing_extern();
        }
    }
    
    /* Block 2: RT_STATIC_ASSERT */
    else if (stage == 2) {
        static_assert(1, "dummy");  /* Valid first */
        (1, "missing static_assert");  /* Error */
    }
    
    #endif
    
    /* Nested function to test scoping */
    {
        /* Local class definition missing 'class' */
        struct LocalStruct {
            int x;
        };
        
        /* Missing 'class' in local context */
        MissingLocalClass {
            int y;
        };
    }
    
    /* Test with attributes affecting parser state */
    __attribute__((unused)) void (*funcptr)() = 0;
    
    /* After attribute, test for missing token */
    if (stage == 3) {
        __attribute__((unused)) class AttrClass {
            int attr_member;
        };
        
        /* Missing 'class' after attribute */
        __attribute__((unused)) MissingAttrClass {
            int missing_attr_member;
        };
    }
    
    /* Complex template context for C++ */
    #ifdef __cplusplus
    if (stage == 4) {
        template<typename T>  /* Valid template */
        class ValidTemplate {
            T value;
        };
        
        <typename U>  /* Missing 'template' */
        class InvalidTemplate {
            U value;
        };
    }
    #endif
    
    /* Final valid return */
    return 0;
}

/* Additional test cases at file scope */
#ifdef __cplusplus
/* Missing 'namespace' at file scope */
file_scope_namespace {
    int file_var = 100;
}

/* Missing 'using' at file scope */
namespace missing_using_ns {
    int ns_var = 200;
}

/* Should be: using namespace missing_using_ns; */
namespace missing_using_ns;

#endif

/* C file-scope tests */
#ifndef __cplusplus
/* Missing 'extern' at file scope */
"C" int file_scope_extern_var = 300;

/* Missing 'asm' at file scope */
volatile ("nop") int asm_var;
#endif
