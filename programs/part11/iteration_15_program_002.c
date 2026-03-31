/* Test for emulated TLS attribute copying - covers lines 295-304 in tree-emutls.cc */

/* Force emulated TLS model */
#pragma GCC tls_model emulated

/* Pattern A: Explicit emulated TLS flag will be used during compilation */

/* DECL_PRESERVE_P - marked as used */
__thread int tls_preserve __attribute__((used)) = 42;

/* TREE_USED - ensure it's referenced */
__thread int tls_used = 100;

/* TREE_PUBLIC - non-static (public) TLS */
__thread int tls_public = 200;

/* DECL_COMMON - tentative definition */
__thread int tls_common;  /* No initializer at file scope */

/* DECL_WEAK - weak symbol */
__thread int tls_weak __attribute__((weak)) = 300;

/* DECL_VISIBILITY_SPECIFIED with hidden visibility */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 400;

/* DECL_VISIBILITY_SPECIFIED with protected visibility */
__thread int tls_protected __attribute__((visibility("protected"))) = 500;

/* Static TLS (not public) */
static __thread int tls_static = 600;

/* DECL_CONTEXT - defined inside a function */
static void func_with_tls(void) {
    static __thread int tls_in_func = 700;
    tls_in_func++;
}

/* Complex usage to ensure processing */
__thread int* tls_ptr;

/* Extern declaration (will be defined in another file) */
extern __thread int tls_extern;

/* Function that uses all TLS variables to ensure TREE_USED is set */
void use_all_tls(void) {
    /* Reference all TLS variables */
    tls_preserve++;
    tls_used += 2;
    tls_public += 3;
    tls_common = tls_weak;
    tls_hidden--;
    tls_protected *= 2;
    tls_static = tls_in_func;  /* Will be defined in func_with_tls */
    
    /* Take address to ensure complex processing */
    tls_ptr = &tls_public;
    
    /* Use in asm to prevent optimization */
    __asm__ volatile ("" : : "r"(&tls_used) : "memory");
}

/* Pattern C: Use in complex expressions */
__thread struct {
    int a;
    int b;
} tls_struct = {1, 2};

/* Weak alias test */
__thread int tls_original = 999;
extern __thread int tls_alias __attribute__((weak, alias("tls_original")));

int main(void) {
    /* Call function to define function-scoped TLS */
    func_with_tls();
    
    /* Use all TLS variables */
    use_all_tls();
    
    /* Complex usage patterns */
    int sum = tls_preserve + tls_used + tls_public + tls_common;
    sum += tls_weak + tls_hidden + tls_protected + tls_static;
    
    /* Use tls_struct */
    sum += tls_struct.a + tls_struct.b;
    
    /* Use extern (will be linked from another file) */
    sum += tls_extern;
    
    /* Use weak alias */
    sum += tls_alias;
    
    /* Print to prevent optimization */
    printf("TLS sum: %d\n", sum);
    
    return sum > 0 ? 0 : 1;
}
