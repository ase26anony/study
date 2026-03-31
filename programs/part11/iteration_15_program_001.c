/* Test for emulated TLS attribute copying - C version */

/* Force emulated TLS for coverage */
#pragma GCC tls_model emulated

/* DECL_PRESERVE_P: marked with __attribute__((used)) */
__thread int tls_preserve __attribute__((used)) = 42;

/* TREE_USED: will be referenced in code */
__thread int tls_used;

/* TREE_PUBLIC: non-static at file scope */
__thread int tls_public = 100;

/* Static TLS variable (not public) */
static __thread int tls_static = 200;

/* DECL_COMMON: tentative definition */
__thread int tls_common;

/* DECL_WEAK: weak symbol */
__thread int tls_weak __attribute__((weak)) = 300;

/* DECL_VISIBILITY: hidden visibility */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 400;

/* DECL_VISIBILITY_SPECIFIED: protected visibility */
__thread int tls_protected __attribute__((visibility("protected"))) = 500;

/* DECL_VISIBILITY_SPECIFIED: internal visibility */
__thread int tls_internal __attribute__((visibility("internal"))) = 600;

/* DECL_CONTEXT: TLS variable inside function */
static void func_with_tls(void) {
    static __thread int tls_in_func = 700;
    tls_in_func++;
}

/* DECL_DLLIMPORT_P: dllimport on supported targets */
#ifdef _WIN32
extern __thread int tls_imported __declspec(dllimport);
#elif defined(__CYGWIN__) || defined(__MINGW32__)
extern __thread int tls_imported __attribute__((dllimport));
#endif

/* Global-dynamic TLS model for contrast */
__thread int tls_global_dynamic __attribute__((tls_model("global-dynamic"))) = 800;

/* Weak alias to test alias handling */
extern __thread int tls_aliased;
__thread int tls_original = 900;
__thread int tls_aliased __attribute__((weak, alias("tls_original")));

/* Complex usage patterns to ensure processing */
static __thread void* tls_address;
static __thread int tls_array[10];

/* Function that uses TLS variables in non-trivial ways */
int use_tls_variables(void) {
    int sum = 0;
    
    /* Ensure TREE_USED is set */
    sum += tls_used;
    tls_used = 10;
    sum += tls_used;
    
    sum += tls_public;
    sum += tls_static;
    sum += tls_common;
    tls_common = 20;
    sum += tls_common;
    
    sum += tls_weak;
    sum += tls_hidden;
    sum += tls_protected;
    sum += tls_internal;
    
    /* Take address to force processing */
    tls_address = &tls_public;
    sum += *(int*)tls_address;
    
    /* Use in array */
    for (int i = 0; i < 10; i++) {
        tls_array[i] = i;
        sum += tls_array[i];
    }
    
    /* Call function with TLS */
    func_with_tls();
    
    sum += tls_global_dynamic;
    sum += tls_original;
    sum += tls_aliased;
    
    return sum;
}

int main(void) {
    int result = use_tls_variables();
    
    /* Use in inline asm to ensure they're not optimized away */
    asm volatile ("" : : "r"(tls_preserve), "r"(tls_used), "r"(tls_public));
    
    return result > 0 ? 0 : 1;
}
