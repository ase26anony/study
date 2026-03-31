/* Test for emulated TLS attribute copying coverage */
#include <stdio.h>
#include <stdint.h>

/* Pattern A: Explicit emulated TLS model */
#ifdef __cplusplus
extern "C" {
#endif

/* DECL_PRESERVE_P - marked as used */
__thread int tls_preserve __attribute__((used)) = 42;
__thread int tls_not_preserve = 0;

/* TREE_USED - ensure variables are referenced */
__thread int tls_used1 = 1;
__thread int tls_used2 = 2;

/* TREE_PUBLIC / DECL_EXTERNAL mix */
__thread int tls_public = 100;           /* public */
static __thread int tls_static = 200;    /* not public */

/* DECL_COMMON - tentative definitions */
__thread int tls_common;                 /* common symbol */

/* DECL_WEAK */
__thread int tls_weak __attribute__((weak)) = 300;

/* DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 400;
__thread int tls_protected __attribute__((visibility("protected"))) = 500;
__thread int tls_internal __attribute__((visibility("internal"))) = 600;
/* Default visibility is implicit */

/* DECL_DLLIMPORT_P - target specific */
#ifdef _WIN32
__declspec(dllimport) extern __thread int tls_imported;
#elif defined(__CYGWIN__) || defined(__MINGW32__)
extern __thread int tls_imported __attribute__((dllimport));
#else
/* On non-Windows, we'll just declare it normally */
extern __thread int tls_imported;
#endif

/* Function to ensure TREE_USED is set */
void use_tls_vars(void) {
    tls_used1 = tls_used2 + 1;
    tls_used2 = tls_used1 * 2;
    
    /* Reference all TLS variables to ensure they're marked used */
    volatile int sum = 0;
    sum += tls_preserve;
    sum += tls_not_preserve;
    sum += tls_public;
    sum += tls_static;
    sum += tls_common;
    sum += tls_weak;
    sum += tls_hidden;
    sum += tls_protected;
    sum += tls_internal;
    
    /* Take address to force more complex handling */
    int* ptr1 = &tls_used1;
    int* ptr2 = &tls_used2;
    
    /* Use in inline asm to prevent optimization */
    __asm__ volatile ("" : : "r"(ptr1), "r"(ptr2));
}

/* DECL_CONTEXT - variables in different scopes */
static void function_scope(void) {
    /* TLS in function scope */
    static __thread int tls_func_scope = 700;
    tls_func_scope++;
}

#ifdef __cplusplus
}
#endif

/* C++ specific tests */
#ifdef __cplusplus
namespace test_namespace {
    /* TLS in namespace scope */
    thread_local int tls_namespace = 800;
    
    class TestClass {
    public:
        /* Static thread-local member */
        static thread_local int tls_member;
        
        /* Non-static thread-local (C++11) */
        thread_local static int tls_static_member;
        
        void member_function() {
            tls_member++;
            tls_static_member++;
        }
    };
    
    /* Define the static members */
    thread_local int TestClass::tls_member = 900;
    thread_local int TestClass::tls_static_member = 1000;
}
#endif

/* Pattern D: Mix with other TLS models */
__thread int tls_global_dynamic __attribute__((tls_model("global-dynamic"))) = 1100;

/* Weak alias test */
__thread int tls_original = 1200;
extern __thread int tls_alias __attribute__((weak, alias("tls_original")));

int main(void) {
    /* Initialize and use all TLS variables */
    use_tls_vars();
    function_scope();
    
    /* Set values */
    tls_common = 1300;
    
    /* Use all variables in calculations */
    int result = 0;
    result += tls_preserve;
    result += tls_not_preserve;
    result += tls_used1;
    result += tls_used2;
    result += tls_public;
    result += tls_static;
    result += tls_common;
    result += tls_weak;
    result += tls_hidden;
    result += tls_protected;
    result += tls_internal;
    
#ifdef __cplusplus
    using namespace test_namespace;
    result += tls_namespace;
    result += TestClass::tls_member;
    result += TestClass::tls_static_member;
    
    TestClass obj;
    obj.member_function();
#endif
    
    result += tls_global_dynamic;
    result += tls_original;
    result += tls_alias;  /* Should be same as tls_original */
    
    /* Try to use imported TLS if available */
#ifdef TLS_IMPORT_DEFINED
    result += tls_imported;
#endif
    
    printf("Result: %d\n", result);
    
    /* Complex expressions with TLS addresses */
    int* volatile ptr_array[] = {
        &tls_preserve,
        &tls_public,
        &tls_common,
        &tls_weak,
        &tls_hidden
    };
    
    /* Force compiler to keep all variables */
    for (int i = 0; i < sizeof(ptr_array)/sizeof(ptr_array[0]); i++) {
        (*ptr_array[i])++;
    }
    
    return 0;
}
