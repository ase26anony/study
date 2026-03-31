/* test_emutls.c - Test program to cover TLS emulation attribute copying */
/* Compile with: gcc -O2 -ftls-model=emulated -fprofile-arcs -ftest-coverage test_emutls.c -o test_emutls_executable */
/* Or for 32-bit: gcc -m32 -O2 -ftls-model=emulated -fprofile-arcs -ftest-coverage test_emutls.c -o test_emutls_executable */

#include <stdio.h>
#include <stddef.h>

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

/* Pattern E: DLL import simulation (for MinGW targets) */
#ifdef __MINGW32__
extern __thread int tls_imported __attribute__((dllimport));
__thread int tls_imported = 500;
#else
/* Fallback for non-Windows: just a regular TLS variable */
__thread int tls_imported = 500;
#endif

/* Helper function to force address taking and prevent optimization */
__attribute__((noinline)) 
static void use_tls_pointers(void *p1, void *p2, void *p3, void *p4, void *p5, void *p6) {
    /* Dummy writes to prevent optimization */
    volatile static int sink;
    sink = (int)(ptrdiff_t)p1;
    sink = (int)(ptrdiff_t)p2;
    sink = (int)(ptrdiff_t)p3;
    sink = (int)(ptrdiff_t)p4;
    sink = (int)(ptrdiff_t)p5;
    sink = (int)(ptrdiff_t)p6;
}

/* Another helper to use TLS values in computation */
__attribute__((noinline))
static int compute_with_tls(int argc) {
    int result = 0;
    
    /* Use all TLS variables in computation */
    result += tls_static_init;
    result += tls_extern;
    result += tls_weak;
    result += (int)tls_hidden;
    result += (int)tls_protected;
    result += tls_imported;
    
    /* Make result dependent on argc */
    return result * argc;
}

int main(int argc, char **argv) {
    int sum = 0;
    
    /* 1. Assign values to TLS variables */
    tls_static_init = argc;
    tls_extern = 'A' + (argc % 26);
    tls_weak = argc * 10;
    tls_hidden = argc * 100L;
    tls_protected = argc * 200L;
    tls_imported = argc * 5;
    
    /* 2. Use TLS values in computation */
    sum = compute_with_tls(argc);
    
    /* 3. Take addresses of all TLS variables */
    use_tls_pointers(
        &tls_static_init,
        &tls_extern,
        &tls_weak,
        &tls_hidden,
        &tls_protected,
        &tls_imported
    );
    
    /* 4. Conditional use based on TLS values */
    if (tls_static_init > 100) {
        tls_hidden += tls_protected;
    }
    
    /* 5. Loop with TLS dependency */
    for (int i = 0; i < tls_weak % 10; i++) {
        tls_imported += i;
    }
    
    /* Final computation to prevent dead code elimination */
    sum += tls_static_init + tls_extern + tls_weak + 
           (int)tls_hidden + (int)tls_protected + tls_imported;
    
    /* Print to prevent optimization */
    printf("Result: %d (argc=%d)\n", sum, argc);
    
    return sum > 100 ? 0 : 1;
}

/* Additional file to test external linkage (compile separately if needed) */
#ifdef MULTI_FILE_TEST
/* In a separate file, you would have: */
/* file2.c: */
__thread char tls_extern;  /* Tentative definition */

/* And compile with: */
/* gcc -c -ftls-model=emulated file2.c */
/* gcc -c -ftls-model=emulated test_emutls.c */
/* gcc -ftls-model=emulated test_emutls.o file2.o -o test */
#endif
