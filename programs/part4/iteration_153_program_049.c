/* test_emutls.c - Test program for TLS emulation attribute copying */

/* Helper function to force address usage */
__attribute__((noinline)) 
static void use_pointer(void *ptr, int val) {
    volatile int *p = (volatile int *)ptr;
    *p = val;
}

/* Pattern A: Static, initialized TLS variable */
static __thread int tls_static_init = 42;

/* Pattern B: Extern, public TLS variable */
extern __thread char tls_extern;
__thread char tls_extern = 'X';  /* Definition */

/* Pattern C: Weak TLS variable */
__thread int tls_weak __attribute__((weak)) = 100;

/* Pattern D: TLS variables with visibility attributes */
__thread long tls_hidden __attribute__((visibility("hidden"))) = 200;
__thread long tls_protected __attribute__((visibility("protected"))) = 300;

/* Pattern E: DLL import simulation (for MinGW/Windows targets) */
#ifdef _WIN32
extern __thread int tls_imported __attribute__((dllimport));
__thread int tls_imported = 400;
#else
/* On non-Windows, use a similar attribute if available */
extern __thread int tls_imported __attribute__((weak));
__thread int tls_imported = 400;
#endif

/* Another TLS variable with common linkage simulation */
__thread double tls_common;  /* Tentative definition */

/* TLS variable used in multiple functions */
__thread int tls_multi_use = 0;

/* Function that uses TLS variables extensively */
__attribute__((noinline))
static int compute_with_tls(int argc) {
    /* Use all TLS variables in computations */
    tls_static_init = argc * 2;
    tls_extern = 'A' + (argc % 26);
    tls_weak = argc * 3;
    tls_hidden = argc * 4;
    tls_protected = argc * 5;
    tls_imported = argc * 6;
    tls_common = argc * 7.0;
    tls_multi_use++;
    
    /* Complex computation using all TLS variables */
    int result = tls_static_init 
                 + (int)tls_extern 
                 + tls_weak 
                 + (int)tls_hidden 
                 + (int)tls_protected 
                 + tls_imported 
                 + (int)tls_common 
                 + tls_multi_use;
    
    return result;
}

/* Another function that takes addresses of TLS variables */
__attribute__((noinline))
static void take_addresses(int seed) {
    /* Force address taking of all TLS variables */
    use_pointer(&tls_static_init, seed + 1);
    use_pointer((void*)&tls_extern, seed + 2);
    use_pointer(&tls_weak, seed + 3);
    use_pointer(&tls_hidden, seed + 4);
    use_pointer(&tls_protected, seed + 5);
    use_pointer(&tls_imported, seed + 6);
    use_pointer(&tls_common, seed + 7);
    use_pointer(&tls_multi_use, seed + 8);
}

/* Main function with control flow dependencies */
int main(int argc, char **argv) {
    volatile int prevent_optimization = argc;
    
    /* Initial use of TLS variables */
    int sum = compute_with_tls(argc);
    
    /* Conditional based on TLS values */
    if (tls_static_init > 10) {
        take_addresses(argc);
        sum += compute_with_tls(argc + 1);
    }
    
    /* Loop using TLS variable */
    for (tls_multi_use = 0; tls_multi_use < 3; tls_multi_use++) {
        sum += tls_weak * tls_multi_use;
    }
    
    /* Use visibility-attribute TLS variables */
    if (tls_hidden != 0) {
        sum += tls_protected / 2;
    }
    
    /* Use DLL-import simulated variable */
    sum += tls_imported;
    
    /* Final computation to prevent dead code elimination */
    int final_result = sum + (int)tls_extern + (int)tls_common;
    
    /* Return value depends on all TLS variables */
    return final_result % 256;
}

/* Additional file-scope TLS usage */
__thread int file_scope_tls = 0;

void __attribute__((constructor)) init_tls(void) {
    file_scope_tls = 12345;
}

/* Force generation of TLS initialization code */
static int dummy = (file_scope_tls = 1, 0);
