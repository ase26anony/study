/* test_emutls.c - Comprehensive TLS emulation test */
#include <stdio.h>
#include <stddef.h>

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

/* Pattern E: DLL import simulation (for MinGW/Windows targets) */
#ifdef _WIN32
extern __thread int tls_imported __attribute__((dllimport));
#else
/* Simulate similar linkage without actual DLL import */
extern __thread int tls_imported;
__thread int tls_imported __attribute__((weak)) = 500;
#endif

/* Additional TLS variables with different storage classes */
__thread volatile int tls_volatile = 999;
static __thread float tls_static_uninit;

/* Noinline helper to force address materialization */
__attribute__((noinline, noipa))
static void use_tls_addresses(void *addr1, void *addr2, void *addr3, 
                              void *addr4, void *addr5, void *addr6) {
    /* Dummy writes to prevent optimization */
    volatile static int sink;
    sink = (addr1 != NULL);
    sink = (addr2 != NULL);
    sink = (addr3 != NULL);
    sink = (addr4 != NULL);
    sink = (addr5 != NULL);
    sink = (addr6 != NULL);
}

/* Another helper that uses TLS values */
__attribute__((noinline, noipa))
static int compute_with_tls(int base) {
    int result = base;
    
    /* Use all TLS variables in computation */
    result += tls_static_init;
    result += tls_extern;
    result += tls_weak;
    result += tls_hidden % 100;
    result += tls_protected % 100;
    result += tls_imported;
    result += tls_volatile % 100;
    result += (int)tls_static_uninit;
    
    return result;
}

/* Function that modifies TLS variables */
__attribute__((noinline, noipa))
static void modify_tls_values(int argc) {
    /* Pattern A usage */
    tls_static_init = argc * 10;
    
    /* Pattern B usage */
    tls_extern = 'A' + (argc % 26);
    
    /* Pattern C usage - only modify if strong definition exists */
    if (&tls_weak != NULL) {
        tls_weak = argc * 20;
    }
    
    /* Pattern D usage */
    tls_hidden = argc * 1000L;
    tls_protected = argc * 2000L;
    
    /* Pattern E usage */
    tls_imported = argc * 50;
    
    /* Other TLS variables */
    tls_volatile = argc * 999;
    tls_static_uninit = argc * 3.14f;
}

int main(int argc, char **argv) {
    int result = 0;
    
    /* Ensure all TLS variables are marked as used */
    if (argc > 1) {
        /* Modify TLS values based on command line */
        modify_tls_values(argc);
    }
    
    /* Compute with TLS values */
    result = compute_with_tls(argc);
    
    /* Take addresses of all TLS variables to force emulation */
    use_tls_addresses(
        &tls_static_init,
        &tls_extern,
        &tls_weak,
        &tls_hidden,
        &tls_protected,
        &tls_imported
    );
    
    /* Also take addresses of other TLS variables */
    use_tls_addresses(
        &tls_volatile,
        &tls_static_uninit,
        NULL, NULL, NULL, NULL
    );
    
    /* Create control flow dependencies on TLS values */
    for (int i = 0; i < tls_static_init % 10; i++) {
        result += i * tls_weak;
    }
    
    /* Use TLS in conditional expressions */
    switch (tls_extern % 4) {
        case 0: result += tls_hidden & 0xFF; break;
        case 1: result += tls_protected & 0xFF; break;
        case 2: result += tls_imported & 0xFF; break;
        case 3: result += tls_volatile & 0xFF; break;
    }
    
    /* Prevent dead code elimination */
    printf("Result: %d (TLS values: %d, %c, %d, %ld, %ld, %d, %d, %f)\n",
           result,
           tls_static_init,
           tls_extern,
           tls_weak,
           tls_hidden,
           tls_protected,
           tls_imported,
           tls_volatile,
           tls_static_uninit);
    
    return result % 256;
}

/* Force generation of TLS initialization code */
__attribute__((constructor))
static void init_tls_test(void) {
    /* Access TLS in constructor to ensure initialization code is generated */
    volatile int dummy = tls_static_init + tls_extern;
    (void)dummy;
}
