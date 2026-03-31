/* Test for emulated TLS attribute copying - covers lines 295-304 in tree-emutls.cc */

/* Force emulated TLS model */
#pragma GCC tls_model emulated

/* Pattern A: Diverse TLS variables with various attributes */

/* DECL_PRESERVE_P - marked as used */
__thread int tls_preserved __attribute__((used)) = 42;

/* TREE_USED - will be referenced in code */
__thread int tls_used_uninitialized;

/* TREE_PUBLIC - non-static at file scope */
__thread int tls_public = 100;

/* Static TLS - not public */
static __thread int tls_static = 200;

/* DECL_COMMON - tentative definition */
__thread int tls_common;

/* DECL_WEAK - weak symbol */
__thread int tls_weak __attribute__((weak)) = 300;

/* DECL_VISIBILITY_SPECIFIED with hidden visibility */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 400;

/* DECL_VISIBILITY_SPECIFIED with protected visibility */
__thread int tls_protected __attribute__((visibility("protected"))) = 500;

/* DECL_VISIBILITY_SPECIFIED with internal visibility */
__thread int tls_internal __attribute__((visibility("internal"))) = 600;

/* Function-scoped TLS - tests DECL_CONTEXT */
void test_function_scope(void) {
    static __thread int tls_function_local = 700;
    tls_function_local++;
}

/* Pattern C: Complex usage to ensure processing */
__thread int* tls_pointer;

/* Pattern D: Contrast with different TLS model */
__thread int tls_global_dynamic __attribute__((tls_model("global-dynamic"))) = 800;

/* Extern declaration - tests DECL_EXTERNAL */
extern __thread int tls_extern;

/* DLL import simulation (conditional) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#elif defined(__MINGW32__) || defined(__CYGWIN__)
__attribute__((dllimport)) __thread int tls_dllimport;
#endif

/* Weak alias test */
__thread int tls_original = 900;
__thread int tls_alias __attribute__((weak, alias("tls_original")));

/* Complex expression usage */
int use_tls_in_asm(void) {
    int result;
    __asm__ volatile (
        "movl %1, %%eax\n\t"
        "addl $1, %%eax\n\t"
        "movl %%eax, %0"
        : "=r" (result)
        : "m" (tls_public)
        : "%eax"
    );
    return result;
}

/* Main function that uses all TLS variables */
int main(void) {
    /* Ensure TREE_USED is set for all variables */
    int sum = 0;
    
    sum += tls_preserved;
    sum += tls_used_uninitialized = 50;  /* Write to ensure used */
    sum += tls_public;
    sum += tls_static;
    sum += tls_common = 150;  /* Initialize the common variable */
    sum += tls_weak;
    sum += tls_hidden;
    sum += tls_protected;
    sum += tls_internal;
    
    test_function_scope();
    
    /* Take address - complex usage */
    tls_pointer = &tls_public;
    sum += *tls_pointer;
    
    sum += tls_global_dynamic;
    
    /* Reference extern (will be defined in another file) */
    sum += tls_extern;
    
    /* Use in non-inlinable function call */
    sum += use_tls_in_asm();
    
    sum += tls_original;
    sum += tls_alias;  /* Should be same as tls_original */
    
    /* Print to prevent optimization */
    printf("TLS sum: %d\n", sum);
    
    return sum > 0 ? 0 : 1;
}
