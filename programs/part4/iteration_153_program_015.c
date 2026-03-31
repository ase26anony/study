/* test_emutls.c - Comprehensive TLS emulation test */
#include <stdio.h>
#include <stddef.h>

/* Pattern A: Static, initialized TLS variable */
static __thread int tls_static_init = 42;

/* Pattern B: Extern, public TLS variable */
extern __thread char tls_extern;
__thread char tls_extern = 'X';  /* Definition */

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

/* Additional TLS variables with different storage classes */
__thread volatile int tls_volatile = 3000;
static __thread float tls_static_uninit;

/* Helper function to force address usage and prevent optimization */
__attribute__((noinline)) 
static void use_tls_addresses(void *addr1, void *addr2, void *addr3, 
                              void *addr4, void *addr5, void *addr6) {
    /* Dummy writes to force materialization of addresses */
    volatile static int sink;
    sink = (addr1 != NULL);
    sink = (addr2 != NULL);
    sink = (addr3 != NULL);
    sink = (addr4 != NULL);
    sink = (addr5 != NULL);
    sink = (addr6 != NULL);
    (void)sink;  /* Suppress unused warning */
}

/* Another helper to use TLS values */
__attribute__((noinline))
static int compute_with_tls(int base) {
    int result = base;
    
    /* Use all TLS variables in computation */
    result += tls_static_init;
    result += tls_extern;
    result += tls_weak;
    result += tls_hidden % 100;
    result += tls_protected % 100;
    result += tls_imported % 100;
    result += tls_volatile % 100;
    result += (int)tls_static_uninit;
    
    return result;
}

/* Function in another compilation unit simulation */
__attribute__((weak))
void external_function(void) {
    /* Access TLS variables that might be defined elsewhere */
    tls_weak = 999;
}

int main(int argc, char *argv[]) {
    int i, sum = 0;
    
    /* 1. Assign values to TLS variables based on argc */
    tls_static_init = argc * 10;
    tls_extern = 'A' + (argc % 26);
    tls_weak = argc * 100;
    tls_hidden = argc * 1000L;
    tls_protected = argc * 2000L;
    tls_imported = argc * 5000;
    tls_volatile = argc * 3000;
    tls_static_uninit = argc * 1.5f;
    
    /* 2. Use TLS variables in computations with control flow */
    for (i = 0; i < argc; i++) {
        if (i % 2 == 0) {
            tls_static_init += i;
            tls_hidden -= i;
        } else {
            tls_protected += i;
            tls_weak -= i;
        }
        
        /* Create data dependencies */
        tls_extern = (tls_extern + 1) > 'Z' ? 'A' : tls_extern + 1;
        tls_volatile = tls_volatile ^ (i * 7);
    }
    
    /* 3. Take addresses of all TLS variables */
    use_tls_addresses(&tls_static_init, &tls_extern, &tls_weak,
                     &tls_hidden, &tls_protected, &tls_imported);
    
    /* Also take addresses of remaining TLS variables */
    use_tls_addresses(&tls_volatile, &tls_static_uninit, 
                     (void*)(ptrdiff_t)tls_static_init,
                     (void*)(ptrdiff_t)tls_extern,
                     (void*)(ptrdiff_t)tls_weak,
                     NULL);
    
    /* 4. Compute final result using TLS values */
    sum = compute_with_tls(argc);
    
    /* 5. Conditional return based on TLS values */
    if (tls_static_init > 1000 || tls_hidden < 0 || tls_protected == 0) {
        return sum % 255;
    }
    
    /* Use all TLS variables one more time to prevent DCE */
    printf("TLS values: %d, %c, %d, %ld, %ld, %d, %d, %f\n",
           tls_static_init, tls_extern, tls_weak,
           tls_hidden, tls_protected, tls_imported,
           tls_volatile, tls_static_uninit);
    
    return (sum + tls_static_init + tls_extern + tls_weak) % 255;
}

/* Additional file simulation for multi-file compilation */
#ifdef COMPILE_SECOND_FILE
/* Second source file to test external linkage */
__thread char tls_extern = 'Y';  /* Different initializer */

void use_tls_in_other_file(void) {
    tls_weak = 888;
    tls_hidden = 7777;
}
#endif
