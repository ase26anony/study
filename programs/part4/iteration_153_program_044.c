/* test_emutls.c - Comprehensive TLS emulation test */
/* Compile with: gcc -O2 -ftls-model=emulated -fprofile-arcs -ftest-coverage test_emutls.c -o test_emutls */

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

/* Pattern E: DLL import simulation (for MinGW/Windows targets) */
#ifdef _WIN32
extern __thread int tls_imported __attribute__((dllimport));
__thread int tls_imported = 5000;
#else
/* Simulate similar behavior with weak attribute */
extern __thread int tls_imported __attribute__((weak));
__thread int tls_imported = 5000;
#endif

/* Additional TLS variables with different storage classes */
__thread volatile int tls_volatile = 99;
__thread const int tls_const = 123;
__thread int tls_uninitialized;

/* Helper function to force address usage */
__attribute__((noinline)) 
static void use_tls_address(void *addr, int size) {
    volatile char *p = (volatile char *)addr;
    /* Dummy write to prevent optimization */
    if (size > 0) {
        *p = *p;  /* Read and write back */
    }
}

/* Another helper to create control flow dependencies */
__attribute__((noinline))
static int compute_checksum(int seed) {
    volatile int sum = seed;
    
    /* Use all TLS variables in computations */
    sum += tls_static_init;
    sum += tls_extern;
    sum += tls_weak;
    sum += tls_hidden % 256;
    sum += tls_protected % 256;
    sum += tls_imported % 256;
    sum += tls_volatile;
    sum += tls_const;
    sum += tls_uninitialized;
    
    return sum;
}

int main(int argc, char **argv) {
    int result = 0;
    
    /* Force initialization and usage of all TLS variables */
    if (argc > 1) {
        /* Pattern A usage */
        tls_static_init = argc * 10;
        
        /* Pattern B usage */
        tls_extern = 'A' + (argc % 26);
        
        /* Pattern C usage */
        tls_weak = argc * 20;
        
        /* Pattern D usage */
        tls_hidden = argc * 100L;
        tls_protected = argc * 200L;
        
        /* Pattern E usage */
        tls_imported = argc * 50;
        
        /* Other TLS variables */
        tls_volatile = argc;
        tls_uninitialized = argc * 2;
    }
    
    /* Take addresses of all TLS variables to force emulation */
    use_tls_address(&tls_static_init, sizeof(tls_static_init));
    use_tls_address(&tls_extern, sizeof(tls_extern));
    use_tls_address(&tls_weak, sizeof(tls_weak));
    use_tls_address(&tls_hidden, sizeof(tls_hidden));
    use_tls_address(&tls_protected, sizeof(tls_protected));
    use_tls_address(&tls_imported, sizeof(tls_imported));
    use_tls_address((void*)&tls_volatile, sizeof(tls_volatile));
    use_tls_address((void*)&tls_const, sizeof(tls_const));
    use_tls_address(&tls_uninitialized, sizeof(tls_uninitialized));
    
    /* Create control flow that depends on TLS values */
    for (int i = 0; i < argc; i++) {
        tls_static_init += i;
        tls_volatile = i;
        
        if (i % 2 == 0) {
            tls_hidden += i;
        } else {
            tls_protected += i;
        }
    }
    
    /* Final computation that uses all TLS variables */
    result = compute_checksum(argc);
    
    /* Prevent dead code elimination */
    if (result > 1000000) {
        /* This branch should never be taken, but prevents optimization */
        return tls_static_init + tls_extern + tls_weak;
    }
    
    return result % 256;
}

/* Additional file to test external linkage (compile separately if needed) */
#ifdef MULTI_FILE_TEST
/* test_emutls2.c */
extern __thread int tls_extern_ref;
__thread int tls_extern_ref = 999;

void use_extern_tls(void) {
    tls_extern_ref = 888;
}
#endif
