/* test_emutls.c - Comprehensive test for TLS emulation attribute copying */
/* Compile with: gcc -m32 -O2 -ftls-model=emulated -fprofile-arcs -ftest-coverage -o test_emutls test_emutls.c */

#include <stdio.h>
#include <stddef.h>

/* Pattern A: Static TLS with initialization */
static __thread int tls_static_init = 42;

/* Pattern B: Extern TLS with common linkage */
extern __thread char tls_extern;
__thread char tls_extern = 'B';

/* Pattern C: Weak TLS symbol */
__thread int tls_weak __attribute__((weak)) = 100;

/* Pattern D: TLS with visibility attributes */
__thread long tls_hidden __attribute__((visibility("hidden"))) = 1000;
__thread long tls_protected __attribute__((visibility("protected"))) = 2000;

/* Pattern E: DLL import simulation (MinGW-style) */
#ifdef __MINGW32__
extern __thread int tls_imported __attribute__((dllimport));
__thread int tls_imported = 500;
#else
/* Fallback for non-MinGW targets */
__thread int tls_imported = 500;
#endif

/* Helper function to force address materialization */
__attribute__((noinline)) 
static void use_tls_address(void *addr, int size) {
    volatile char *p = (volatile char *)addr;
    /* Dummy write to prevent optimization */
    if (size > 0) {
        *p = *p;
    }
}

/* Another helper to prevent dead code elimination */
__attribute__((noinline))
static int compute_checksum(int argc) {
    int sum = 0;
    
    /* Use all TLS variables in computations */
    sum += tls_static_init;
    sum += tls_extern;
    sum += tls_weak;
    sum += tls_hidden % 100;
    sum += tls_protected % 100;
    sum += tls_imported;
    
    /* Make result dependent on argc */
    return sum * (argc + 1);
}

int main(int argc, char **argv) {
    int result = 0;
    
    /* Pattern A usage */
    tls_static_init = argc * 10;
    use_tls_address(&tls_static_init, sizeof(tls_static_init));
    
    /* Pattern B usage */
    tls_extern = 'A' + (argc % 26);
    use_tls_address(&tls_extern, sizeof(tls_extern));
    
    /* Pattern C usage - conditional based on weak symbol */
    if (&tls_weak != NULL) {
        tls_weak = argc * 20;
        use_tls_address(&tls_weak, sizeof(tls_weak));
    }
    
    /* Pattern D usage */
    tls_hidden = argc * 100L;
    tls_protected = argc * 200L;
    use_tls_address(&tls_hidden, sizeof(tls_hidden));
    use_tls_address(&tls_protected, sizeof(tls_protected));
    
    /* Pattern E usage */
    tls_imported = argc * 50;
    use_tls_address(&tls_imported, sizeof(tls_imported));
    
    /* Complex computation using all TLS variables */
    for (int i = 0; i < argc; i++) {
        tls_static_init += i;
        tls_extern = (tls_extern + 1) % 128;
        result += compute_checksum(i);
    }
    
    /* Final result depends on all TLS variables */
    result += tls_static_init;
    result += tls_extern;
    result += tls_weak;
    result += tls_hidden % 256;
    result += tls_protected % 256;
    result += tls_imported;
    
    /* Print to prevent optimization */
    printf("Result: %d\n", result);
    
    return result % 256;
}

/* Additional file-scope TLS to test more cases */
__thread int tls_file_scope = 300;
static __thread int tls_file_static = 400;

/* Function in another compilation unit simulation */
__attribute__((weak))
__thread int tls_tentative;  /* Tentative definition */

/* Usage of file-scope TLS */
__attribute__((constructor))
static void init_tls_values(void) {
    tls_file_scope = 600;
    tls_file_static = 700;
    tls_tentative = 800;
}
