/* test_emutls.c - Comprehensive TLS emulation test */
/* Compile with: gcc -O2 -ftls-model=emulated -fprofile-arcs -ftest-coverage test_emutls.c -o test_emutls */

#include <stdio.h>
#include <stdint.h>

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

/* Pattern E: DLL import simulation (for MinGW/Cygwin targets) */
#ifdef _WIN32
extern __thread int tls_imported __attribute__((dllimport));
__thread int tls_imported = 400;
#else
/* Simulate similar behavior with weak attribute */
extern __thread int tls_imported __attribute__((weak));
__thread int tls_imported = 400;
#endif

/* Additional TLS variables with different types and qualifiers */
__thread volatile double tls_volatile = 3.14159;
__thread const char* tls_pointer = "TLS String";
static __thread unsigned long tls_static_uninit;

/* Function to ensure addresses are taken and used */
__attribute__((noinline)) 
static void use_tls_addresses(void* addr1, void* addr2, void* addr3, 
                              void* addr4, void* addr5, void* addr6) {
    /* Dummy writes to prevent optimization */
    volatile static int sink = 0;
    sink += (int)((uintptr_t)addr1 ^ (uintptr_t)addr2);
    sink += (int)((uintptr_t)addr3 ^ (uintptr_t)addr4);
    sink += (int)((uintptr_t)addr5 ^ (uintptr_t)addr6);
    (void)sink; /* Suppress unused warning */
}

/* Another noinline function to use TLS values */
__attribute__((noinline))
static int compute_with_tls(int base) {
    int result = base;
    
    /* Use all TLS variables in computation */
    result += tls_static_init;
    result += tls_extern;
    result += tls_weak;
    result += (int)tls_hidden;
    result += (int)tls_protected;
    result += tls_imported;
    result += (int)tls_volatile;
    result += (int)(uintptr_t)tls_pointer;
    result += (int)tls_static_uninit;
    
    /* Create control flow dependencies */
    if (tls_static_init > 0) {
        result *= 2;
    }
    
    for (int i = 0; i < (tls_weak % 5); i++) {
        result += i;
    }
    
    return result;
}

/* Function to modify TLS variables */
__attribute__((noinline))
static void modify_tls_vars(int argc) {
    /* Modify Pattern A */
    tls_static_init = argc * 10;
    
    /* Modify Pattern B */
    tls_extern = 'A' + (argc % 26);
    
    /* Modify Pattern C (if strong definition exists) */
    tls_weak = argc * 20;
    
    /* Modify Pattern D */
    tls_hidden = argc * 30L;
    tls_protected = argc * 40L;
    
    /* Modify Pattern E */
    tls_imported = argc * 50;
    
    /* Modify other TLS variables */
    tls_volatile = argc * 3.14159;
    tls_static_uninit = argc * 60UL;
    
    /* tls_pointer remains unchanged (const) */
}

int main(int argc, char** argv) {
    int result = 0;
    
    /* Ensure TLS variables are marked as used */
    TREE_USED(&tls_static_init); /* This is a compiler internal macro - 
                                  we'll actually use the variables below */
    
    /* Take addresses of all TLS variables */
    void* addrs[] = {
        &tls_static_init,
        &tls_extern,
        &tls_weak,
        &tls_hidden,
        &tls_protected,
        &tls_imported,
        &tls_volatile,
        (void*)&tls_pointer,
        &tls_static_uninit
    };
    
    /* Use addresses to prevent optimization */
    use_tls_addresses(addrs[0], addrs[1], addrs[2], addrs[3], addrs[4], addrs[5]);
    use_tls_addresses(addrs[6], addrs[7], addrs[8], addrs[0], addrs[1], addrs[2]);
    
    /* Modify TLS variables based on program input */
    modify_tls_vars(argc);
    
    /* Compute result using TLS variables */
    result = compute_with_tls(argc);
    
    /* Create side effects to prevent dead code elimination */
    printf("Result: %d\n", result);
    printf("TLS values: %d, %c, %d, %ld, %ld, %d\n",
           tls_static_init, tls_extern, tls_weak,
           tls_hidden, tls_protected, tls_imported);
    
    /* Return value depends on TLS state */
    return result % 256;
}

/* Additional file-like separation using weak symbols */
/* This tests DECL_EXTERNAL and DECL_COMMON behavior */
__thread int tls_tentative;  /* Tentative definition - tests DECL_COMMON */

/* Another translation unit simulation using weak linkage */
extern __thread int tls_other_unit __attribute__((weak));
__thread int tls_other_unit = 999;

/* Force generation of TLS emulation structures */
/* The volatile asm prevents optimization of TLS accesses */
__attribute__((noinline)) static void force_tls_emulation(void) {
    volatile __thread int force_tls = 123;
    int local = force_tls;
    (void)local;
}
