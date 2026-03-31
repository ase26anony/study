/* tls_emulation_test.c - Test GCC's emulated TLS attribute propagation */

/* Force emulated TLS even if platform supports native TLS */
#pragma GCC target("tls")

#include <stdio.h>
#include <stdint.h>

/* Prevent inlining to force TLS address usage */
#define NOINLINE __attribute__((noinline))
#define CONSTRUCTOR __attribute__((constructor))
#define DESTRUCTOR __attribute__((destructor))

/* ===== TLS VARIABLES WITH DIVERSE ATTRIBUTES ===== */

/* 1. Weak TLS variable with hidden visibility */
__thread int tls_weak_hidden __attribute__((weak, visibility("hidden"))) = 42;

/* 2. Public TLS variable with default visibility */
__thread int tls_public_default = 100;

/* 3. Protected visibility TLS variable */
__thread int tls_protected __attribute__((visibility("protected"))) = 200;

/* 4. Common linkage TLS (tentative definition) */
__thread int tls_common;

/* 5. External TLS declaration (defined in another TU) */
extern __thread int tls_external;

/* 6. DLL import style (simulated with weak) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#else
__thread int tls_dllimport __attribute__((weak));
#endif

/* 7. Static TLS inside a function context */
static void func_with_static_tls(void) {
    static __thread int tls_func_static = 300;
    volatile int* p = &tls_func_static;
    *p += 1;
}

/* 8. TLS with preserve attribute (via used) */
__thread int tls_preserved __attribute__((used)) = 400;

/* ===== HELPER FUNCTIONS THAT TAKE TLS ADDRESSES ===== */

NOINLINE static void use_tls_weak_hidden(volatile int** ptr) {
    *ptr = &tls_weak_hidden;
    **ptr += 1;  /* Force memory access */
}

NOINLINE static void use_tls_public_default(volatile int** ptr) {
    *ptr = &tls_public_default;
    **ptr += 2;
}

NOINLINE static void use_tls_protected(volatile int** ptr) {
    *ptr = &tls_protected;
    **ptr += 3;
}

NOINLINE static void use_tls_common(volatile int** ptr) {
    *ptr = &tls_common;
    **ptr = 123;  /* Initialize common TLS */
}

NOINLINE static void use_tls_dllimport(volatile int** ptr) {
    *ptr = &tls_dllimport;
    if (*ptr != NULL) {
        **ptr += 4;
    }
}

/* ===== CONSTRUCTOR/DESTRUCTOR INTERACTIONS ===== */

CONSTRUCTOR static void init_tls_in_constructor(void) {
    /* Access and modify TLS in constructor */
    volatile int* p = &tls_preserved;
    *p = 999;
    
    /* Also touch common TLS */
    tls_common = 456;
    
    /* Force TREE_USED flag by taking address */
    volatile int* unused __attribute__((unused)) = &tls_weak_hidden;
}

DESTRUCTOR static void check_tls_in_destructor(void) {
    /* Verify TLS is still accessible */
    volatile int check = tls_preserved;
    (void)check;
}

/* ===== COMPLEX CONTROL FLOW WITH TLS ===== */

NOINLINE static uint32_t compute_tls_checksum(void) {
    volatile uint32_t sum = 0;
    volatile int* ptrs[6];
    volatile int selector = 0;
    
    /* Initialize pointer array with TLS addresses */
    ptrs[0] = &tls_weak_hidden;
    ptrs[1] = &tls_public_default;
    ptrs[2] = &tls_protected;
    ptrs[3] = &tls_common;
    ptrs[4] = &tls_dllimport;
    ptrs[5] = &tls_preserved;
    
    /* Complex loop with volatile control to prevent optimization */
    for (volatile int i = 0; i < 100; i++) {
        selector = (selector * 1103515245 + 12345) & 0x7FFFFFFF;
        int idx = selector % 6;
        
        /* Access TLS through volatile pointer */
        sum += *ptrs[idx];
        
        /* Modify TLS occasionally */
        if ((selector & 0xFF) == 0) {
            *ptrs[idx] += 1;
        }
    }
    
    return sum;
}

/* ===== MAIN EXECUTION FLOW ===== */

int main(void) {
    volatile uint32_t checksum = 0;
    volatile int* tmp_ptr;
    
    /* 1. Call all helper functions to force TLS address usage */
    use_tls_weak_hidden(&tmp_ptr);
    use_tls_public_default(&tmp_ptr);
    use_tls_protected(&tmp_ptr);
    use_tls_common(&tmp_ptr);
    use_tls_dllimport(&tmp_ptr);
    
    /* 2. Call function with static TLS */
    func_with_static_tls();
    
    /* 3. Direct TLS accesses with volatile qualifiers */
    volatile int* volatile p1 = &tls_weak_hidden;
    volatile int* volatile p2 = &tls_public_default;
    volatile int* volatile p3 = &tls_protected;
    
    *p1 += *p2;
    *p3 = *p1 ^ *p2;
    
    /* 4. Conditional TLS access based on runtime value */
    volatile int mode = 0;
    for (volatile int i = 0; i < 10; i++) {
        mode = (mode + i) % 3;
        volatile int* target = NULL;
        
        switch (mode) {
            case 0: target = &tls_weak_hidden; break;
            case 1: target = &tls_public_default; break;
            case 2: target = &tls_protected; break;
        }
        
        if (target) {
            *target += i;
        }
    }
    
    /* 5. Compute final checksum */
    checksum = compute_tls_checksum();
    
    /* 6. Print something to prevent optimization */
    printf("TLS checksum: %u\n", (unsigned)checksum);
    
    /* 7. Verify emulated TLS structure by checking addresses */
    printf("TLS addresses:\n");
    printf("  weak_hidden: %p\n", (void*)&tls_weak_hidden);
    printf("  public_default: %p\n", (void*)&tls_public_default);
    printf("  protected: %p\n", (void*)&tls_protected);
    printf("  common: %p\n", (void*)&tls_common);
    printf("  preserved: %p\n", (void*)&tls_preserved);
    
    return (checksum != 0) ? 0 : 1;
}

/* ===== EXTERNAL TLS DEFINITION (SIMULATED) ===== */
/* In a real test, this would be in a separate compilation unit */
__thread int tls_external = 500;
