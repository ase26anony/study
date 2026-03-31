/* test_emutls.c - Comprehensive TLS emulation test */
#include <stdio.h>
#include <stdint.h>

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

/* Additional TLS variables with different storage classes */
__thread volatile int tls_volatile = 99;
__thread const int tls_const = 123;
__thread int tls_uninit;

/* Helper function to force address usage and prevent optimization */
__attribute__((noinline)) 
static void use_tls_addresses(void *addr1, void *addr2, void *addr3, 
                              void *addr4, void *addr5, void *addr6) {
    volatile static int sink = 0;
    sink += (int)((uintptr_t)addr1 ^ (uintptr_t)addr2);
    sink += (int)((uintptr_t)addr3 ^ (uintptr_t)addr4);
    sink += (int)((uintptr_t)addr5 ^ (uintptr_t)addr6);
    (void)sink;  /* Prevent unused variable warning */
}

/* Another helper that uses TLS values */
__attribute__((noinline))
static int compute_with_tls(int base) {
    int result = base;
    result += tls_static_init;
    result += tls_extern;
    result += tls_weak;
    result += tls_hidden % 256;
    result += tls_protected % 256;
    result += tls_imported % 256;
    result += tls_volatile;
    result += tls_const;
    return result;
}

/* Function to ensure TLS variables are used across translation units */
__attribute__((noinline))
static void modify_tls_values(int argc) {
    /* Modify Pattern A */
    tls_static_init = argc * 10;
    
    /* Modify Pattern B */
    tls_extern = 'A' + (argc % 26);
    
    /* Modify Pattern C (if not weak) */
    if (&tls_weak) {
        tls_weak = argc * 20;
    }
    
    /* Modify Pattern D */
    tls_hidden = 1000 + argc * 100;
    tls_protected = 2000 + argc * 200;
    
    /* Modify Pattern E */
    tls_imported = 5000 + argc * 500;
    
    /* Modify other TLS variables */
    tls_volatile = 99 + argc;
    /* tls_const is const, can't modify */
    tls_uninit = argc * 30;
}

int main(int argc, char **argv) {
    int result = 0;
    
    /* Force initialization and usage of all TLS variables */
    modify_tls_values(argc);
    
    /* Use TLS values in computation */
    result = compute_with_tls(argc);
    
    /* Take addresses of all TLS variables to force emulation structures */
    use_tls_addresses(
        &tls_static_init,
        &tls_extern,
        &tls_weak,
        &tls_hidden,
        &tls_protected,
        &tls_imported
    );
    
    /* Additional address usage */
    use_tls_addresses(
        &tls_volatile,
        &tls_const,
        &tls_uninit,
        &tls_static_init,  /* Reuse to ensure all are referenced */
        &tls_extern,
        &tls_weak
    );
    
    /* Create control flow dependencies on TLS values */
    for (int i = 0; i < (tls_static_init % 8); i++) {
        result += tls_extern;
    }
    
    if (tls_weak > 50) {
        result += tls_hidden;
    }
    
    switch (tls_protected % 4) {
        case 0: result += 1; break;
        case 1: result += 2; break;
        case 2: result += 3; break;
        case 3: result += 4; break;
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    return result % 256;
}

/* Additional file-scope TLS usage to ensure DECL_CONTEXT is set */
__thread int file_scope_tls = 999;

static void __attribute__((constructor)) init_tls(void) {
    file_scope_tls = 888;
}

static void __attribute__((destructor)) cleanup_tls(void) {
    /* Access TLS in destructor */
    volatile int dummy = file_scope_tls;
    (void)dummy;
}
