/* test_emutls.c - Comprehensive TLS emulation test */
#include <stdio.h>

/* Pattern A: Static, initialized TLS variable */
static __thread int tls_static_init = 42;

/* Pattern B: Extern/public TLS variable */
extern __thread char tls_extern;
__thread char tls_extern = 'B';

/* Pattern C: Weak TLS variable */
__thread int tls_weak __attribute__((weak)) = 100;

/* Pattern D: TLS variables with visibility attributes */
__thread long tls_hidden __attribute__((visibility("hidden"))) = 1000;
__thread long tls_protected __attribute__((visibility("protected"))) = 2000;

/* Pattern E: DLL import attribute (simulated for coverage) */
#ifdef _WIN32
extern __thread int tls_imported __attribute__((dllimport));
#else
/* On non-Windows, use a similar attribute if available, or just regular TLS */
extern __thread int tls_imported;
#endif
__thread int tls_imported = 999;

/* Additional TLS variable with no special attributes for baseline */
__thread double tls_regular = 3.14159;

/* Helper function to force address usage and prevent optimization */
__attribute__((noinline)) 
static void use_tls_addresses(void *addr1, void *addr2, void *addr3, 
                              void *addr4, void *addr5, void *addr6) {
    volatile static int sink = 0;
    /* Dummy operations to use the addresses */
    if (addr1) sink += 1;
    if (addr2) sink += 2;
    if (addr3) sink += 3;
    if (addr4) sink += 4;
    if (addr5) sink += 5;
    if (addr6) sink += 6;
    /* Prevent the compiler from optimizing away the function */
    __asm__ __volatile__("" : : "r"(sink) : "memory");
}

/* Another helper that returns a value based on TLS variables */
__attribute__((noinline))
static int compute_from_tls(int arg) {
    int result = 0;
    
    /* Use all TLS variables in computations */
    result += tls_static_init * arg;
    result += tls_extern;
    result += tls_weak;
    result += tls_hidden % 100;
    result += tls_protected % 100;
    result += tls_imported;
    result += (int)tls_regular;
    
    /* Conditional usage to prevent dead code elimination */
    if (result > 1000) {
        tls_static_init = result % 100;
    } else {
        tls_weak = result;
    }
    
    return result;
}

int main(int argc, char **argv) {
    int i, sum = 0;
    
    /* 1. Assign values to TLS variables based on program input */
    tls_static_init = argc * 10;
    tls_extern = 'A' + (argc % 26);
    tls_weak = argc * 20;
    tls_hidden = argc * 30L;
    tls_protected = argc * 40L;
    tls_imported = argc * 50;
    tls_regular = argc * 3.14;
    
    /* 2. Use TLS variables in computations */
    for (i = 0; i < argc; i++) {
        sum += compute_from_tls(i);
        
        /* Modify TLS variables in loop */
        if (i % 2 == 0) {
            tls_static_init += i;
            tls_extern += 1;
        } else {
            tls_weak -= i;
            tls_hidden += i;
        }
    }
    
    /* 3. Take addresses of all TLS variables */
    use_tls_addresses(&tls_static_init, &tls_extern, &tls_weak,
                     &tls_hidden, &tls_protected, &tls_imported);
    
    /* 4. Use addresses again in a different way */
    volatile __thread int *ptrs[] = {
        &tls_static_init,
        &tls_weak,
        &tls_imported,
        (int*)&tls_hidden,
        (int*)&tls_protected,
        NULL
    };
    
    for (i = 0; ptrs[i] != NULL; i++) {
        *ptrs[i] += sum % 100;
    }
    
    /* 5. Final computation using all TLS variables */
    int final_result = 
        tls_static_init + 
        tls_extern + 
        tls_weak + 
        (tls_hidden % 256) + 
        (tls_protected % 256) + 
        tls_imported + 
        (int)tls_regular;
    
    /* Print to prevent optimization */
    printf("TLS test result: %d (argc=%d)\n", final_result, argc);
    
    return final_result > 100 ? 0 : 1;
}

/* Additional file-like separation using weak symbols */
/* This creates a scenario similar to multi-file compilation */
__thread int tls_tentative __attribute__((weak));  /* Tentative definition */

/* Function that uses the weak TLS variable */
__attribute__((noinline)) 
static void use_weak_tls(void) {
    if (&tls_tentative != NULL) {
        tls_tentative = 1234;
    }
}
