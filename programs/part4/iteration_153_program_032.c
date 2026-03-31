/* test_emutls.c - Test program for TLS emulation attribute copying */

/* Force emulated TLS model */
#pragma GCC tls_model emulated

/* Pattern A: Static, initialized TLS variable */
static __thread int tls_static_init = 42;

/* Pattern B: Extern, public TLS variable */
extern __thread char tls_extern;
__thread char tls_extern = 'B';

/* Pattern C: Weak TLS variable */
__thread int tls_weak __attribute__((weak)) = 100;

/* Pattern D: TLS variables with visibility attributes */
__thread long tls_hidden __attribute__((visibility("hidden"))) = 1000;
__thread long tls_protected __attribute__((visibility("protected"))) = 2000;

/* Pattern E: DLL import attribute (simulated for MinGW/Cygwin) */
#ifdef __MINGW32__
extern __thread int tls_imported __attribute__((dllimport));
__thread int tls_imported = 5000;
#else
/* For non-Windows, use a different attribute that might trigger similar handling */
__thread int tls_imported __attribute__((weak, visibility("default"))) = 5000;
#endif

/* Additional TLS variables to ensure coverage */
__thread volatile int tls_volatile = 123;
__thread double tls_double = 3.14159;

/* Helper function to force address taking and prevent optimization */
__attribute__((noinline, noipa))
static void use_tls_addresses(void *addr1, void *addr2, void *addr3, 
                              void *addr4, void *addr5, void *addr6) {
    /* Dummy operations to prevent optimization */
    volatile static int sink = 0;
    sink += (int)((long)addr1 ^ (long)addr2 ^ (long)addr3 ^ 
                  (long)addr4 ^ (long)addr5 ^ (long)addr6);
}

/* Another helper to use TLS values */
__attribute__((noinline, noipa))
static int compute_with_tls(int arg) {
    int result = tls_static_init + tls_weak + (int)tls_extern;
    result += (int)tls_hidden + (int)tls_protected + tls_imported;
    result += tls_volatile + (int)tls_double;
    return result * arg;
}

/* Function in another compilation unit simulation */
__attribute__((weak))
void external_function(void) {
    /* Access TLS variables to force external references */
    tls_static_init = 99;
    tls_extern = 'X';
}

int main(int argc, char **argv) {
    int i, sum = 0;
    
    /* Modify all TLS variables based on argc to create variability */
    tls_static_init = argc * 10;
    tls_extern = 'A' + (argc % 26);
    tls_weak = argc * 20;
    tls_hidden = argc * 30L;
    tls_protected = argc * 40L;
    tls_imported = argc * 50;
    tls_volatile = argc * 60;
    tls_double = argc * 3.14;
    
    /* Use TLS variables in computations */
    for (i = 0; i < argc; i++) {
        sum += tls_static_init;
        sum += tls_weak;
        sum += (int)tls_extern;
        if (i % 2 == 0) {
            tls_hidden += i;
            tls_protected -= i;
        }
    }
    
    /* Force address taking of all TLS variables */
    use_tls_addresses(&tls_static_init, &tls_extern, &tls_weak,
                      &tls_hidden, &tls_protected, &tls_imported);
    
    /* Additional address taking for remaining variables */
    use_tls_addresses(&tls_volatile, &tls_double, &tls_static_init,
                      &tls_extern, &tls_weak, &tls_hidden);
    
    /* Compute with TLS values */
    int computed = compute_with_tls(argc);
    
    /* Create control flow dependency on TLS values */
    if (tls_static_init > 100) {
        tls_weak += tls_hidden;
    } else {
        tls_protected += tls_imported;
    }
    
    /* Final result depends on all TLS variables */
    int final_result = sum + computed + tls_volatile + (int)tls_double;
    
    /* Prevent dead code elimination */
    volatile int output = final_result;
    
    return output % 256;
}

/* Additional file-like separation using weak symbols */
__attribute__((weak))
__thread int tls_common;  /* Tests DECL_COMMON */

/* Force multiple initializations */
__attribute__((constructor))
static void init_tls_values(void) {
    tls_static_init = 4242;
    tls_extern = 'Z';
    tls_weak = 200;
    tls_hidden = 3000;
    tls_protected = 4000;
    tls_imported = 6000;
    tls_volatile = 456;
    tls_double = 2.71828;
}
