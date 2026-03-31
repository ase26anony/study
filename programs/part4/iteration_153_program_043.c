/* test_emutls.c - Comprehensive TLS emulation test */

/* Force TLS emulation even on targets with native TLS support */
#pragma GCC tls_model emulated

/* Pattern A: Static, initialized TLS variable */
static __thread int tls_static_init = 42;

/* Pattern B: Extern, public TLS variable */
extern __thread char tls_extern;
__thread char tls_extern = 'B';  /* Definition */

/* Pattern C: Weak TLS variable */
__thread int tls_weak __attribute__((weak)) = 100;

/* Pattern D: TLS variables with visibility attributes */
__thread long tls_hidden __attribute__((visibility("hidden"))) = 1000;
__thread long tls_protected __attribute__((visibility("protected"))) = 2000;

/* Pattern E: DLL import simulation (for MinGW/Cygwin targets) */
#ifdef __MINGW32__
extern __thread int tls_imported __attribute__((dllimport));
__thread int tls_imported = 5000;
#else
/* Fallback for non-Windows targets */
__thread int tls_imported = 5000;
#endif

/* Pattern F: Common linkage (tentative definition) */
__thread double tls_common;  /* No initializer = common linkage */

/* Pattern G: Preserved variable (used in inline assembly) */
__thread int tls_preserved __asm__("custom_tls_var") = 99;

/* Helper function to force address taking and prevent optimization */
__attribute__((noinline, noipa))
void use_tls_addresses(void *addr1, void *addr2, void *addr3, 
                       void *addr4, void *addr5, void *addr6, void *addr7) {
    /* Dummy volatile writes to prevent optimization */
    volatile static int sink;
    sink = (int)((long)addr1 ^ (long)addr2 ^ (long)addr3 ^ 
                 (long)addr4 ^ (long)addr5 ^ (long)addr6 ^ (long)addr7);
}

/* Another helper to ensure TLS variables are used in computations */
__attribute__((noinline))
int compute_with_tls(int argc) {
    /* Use all TLS variables in computation */
    int result = tls_static_init;
    result += tls_extern;
    result += tls_weak;
    result += tls_hidden % 256;
    result += tls_protected % 256;
    result += tls_imported % 256;
    result += (int)tls_common;
    result += tls_preserved;
    
    /* Make result dependent on argc */
    return result * (argc + 1);
}

/* Function to test DECL_EXTERNAL by referencing before definition */
void reference_external_tls(void) {
    /* Forward reference to force external declaration */
    extern __thread float tls_forward_ref;
    volatile float *p = &tls_forward_ref;
    (void)p;
}

/* Definition of forward-referenced TLS variable */
__thread float tls_forward_ref = 3.14f;

int main(int argc, char **argv) {
    int i;
    
    /* Ensure all TLS variables are marked as used */
    TREE_USED: /* Label to prevent unused variable warnings */
    
    /* Modify all TLS variables based on argc */
    tls_static_init = argc * 10;
    tls_extern = 'A' + (argc % 26);
    tls_weak = argc * 20;
    tls_hidden = argc * 30;
    tls_protected = argc * 40;
    tls_imported = argc * 50;
    tls_common = argc * 3.14159;
    tls_preserved = argc * 60;
    tls_forward_ref = argc * 2.71828f;
    
    /* Use TLS variables in loop with side effects */
    volatile int sum = 0;
    for (i = 0; i < argc; i++) {
        tls_static_init += i;
        tls_weak -= i;
        sum += tls_static_init + tls_weak;
    }
    
    /* Take addresses of all TLS variables */
    use_tls_addresses(&tls_static_init,
                     &tls_extern,
                     &tls_weak,
                     &tls_hidden,
                     &tls_protected,
                     &tls_imported,
                     &tls_common);
    
    /* Also take address of preserved TLS variable */
    volatile void *preserved_addr = &tls_preserved;
    (void)preserved_addr;
    
    /* Reference external TLS */
    reference_external_tls();
    
    /* Final computation using all TLS variables */
    int result = compute_with_tls(argc);
    
    /* Use result in a way that can't be optimized away */
    if (result > 1000000) {
        /* This branch should never be taken, but prevents optimization */
        return -1;
    }
    
    return result % 256;
}

/* Additional file-scope TLS variable to test DECL_CONTEXT */
namespace {
    __thread int tls_anon_namespace = 12345;
}

/* TLS variable in function scope (GNU extension) */
void function_with_local_tls(void) {
    static __thread int local_func_tls = 999;
    local_func_tls++;
    volatile int *p = &local_func_tls;
    (void)p;
}

/* Call it from main to ensure it's used */
void ensure_local_tls_used(void) {
    function_with_local_tls();
}
