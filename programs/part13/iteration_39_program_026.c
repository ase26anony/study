/* tls_emulation_test.c - Test GCC's emulated TLS attribute propagation */
/* Compile with: gcc -O2 -femulated-tls -fno-common -fvisibility=hidden tls_emulation_test.c -o tls_test */

#include <stdio.h>
#include <stdint.h>

/* Prevent inlining to force TLS address usage */
#define NOINLINE __attribute__((noinline))
#define CONSTRUCTOR __attribute__((constructor))
#define DESTRUCTOR __attribute__((destructor))

/* ===== TLS VARIABLES WITH DIVERSE ATTRIBUTES ===== */

/* 1. Weak TLS variable with hidden visibility */
__thread int tls_weak_hidden __attribute__((weak, visibility("hidden"))) = 100;

/* 2. Public TLS variable with default visibility */
__thread int tls_public_default = 200;

/* 3. Common linkage TLS (tentative definition) */
__thread int tls_common __attribute__((common));

/* 4. Protected visibility TLS */
__thread int tls_protected __attribute__((visibility("protected"))) = 300;

/* 5. DLL import simulation (using weak external) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#else
extern __thread int tls_dllimport __attribute__((weak));
#endif

/* 6. Static TLS inside function will be tested separately */
/* 7. External declaration (defined later) */
extern __thread int tls_external;

/* 8. TLS with preserved flag (used in constructor) */
__thread int tls_preserved __attribute__((used)) = 400;

/* ===== FUNCTION DECLARATIONS ===== */
NOINLINE static void use_tls_address(int *addr);
NOINLINE static void modify_tls_volatile(volatile int *addr);
NOINLINE static int compute_tls_checksum(void);

/* ===== TLS VARIABLE DEFINITIONS ===== */
/* Define the external TLS variable */
__thread int tls_external = 500;

/* ===== HELPER FUNCTIONS ===== */

NOINLINE static void use_tls_address(int *addr) {
    /* Force TLS variable to be marked as used */
    static volatile int sink;
    sink = *addr;
}

NOINLINE static void modify_tls_volatile(volatile int *addr) {
    /* Volatile access prevents optimization */
    *addr = *addr + 1;
}

NOINLINE static int compute_tls_checksum(void) {
    /* Compute checksum ensuring all TLS variables are accessed */
    int sum = 0;
    
    /* Access through volatile pointers */
    volatile int *v1 = &tls_weak_hidden;
    volatile int *v2 = &tls_public_default;
    volatile int *v3 = &tls_common;
    volatile int *v4 = &tls_protected;
    volatile int *v5 = &tls_external;
    volatile int *v6 = &tls_preserved;
    
    sum += *v1;
    sum += *v2;
    sum += *v3;
    sum += *v4;
    sum += *v5;
    sum += *v6;
    
    return sum;
}

/* ===== CONSTRUCTOR/DESTRUCTOR INTERACTION ===== */

CONSTRUCTOR static void init_tls_constructor(void) {
    /* This tests DECL_PRESERVE_P propagation */
    tls_preserved = 999;
    tls_common = 777;  /* Initialize common TLS */
    
    /* Take addresses to ensure they're marked used */
    use_tls_address(&tls_preserved);
    use_tls_address(&tls_common);
}

DESTRUCTOR static void cleanup_tls_destructor(void) {
    /* Final access to TLS variables */
    volatile int final = tls_preserved + tls_public_default;
    (void)final;
}

/* ===== FUNCTION WITH STATIC TLS ===== */

NOINLINE static void function_with_static_tls(void) {
    /* Static TLS inside function - different DECL_CONTEXT */
    static __thread int tls_static_func = 600;
    
    /* Take address and modify */
    int *addr = &tls_static_func;
    modify_tls_volatile((volatile int *)addr);
    
    /* Use in conditional */
    if (tls_static_func > 0) {
        use_tls_address(&tls_static_func);
    }
}

/* ===== MAIN TEST FUNCTION ===== */

int main(void) {
    volatile int selector = 0;
    int checksum = 0;
    
    /* 1. Take addresses of all TLS variables */
    use_tls_address(&tls_weak_hidden);
    use_tls_address(&tls_public_default);
    use_tls_address(&tls_common);
    use_tls_address(&tls_protected);
    use_tls_address(&tls_external);
    use_tls_address(&tls_preserved);
    
    /* 2. Call function with static TLS */
    function_with_static_tls();
    
    /* 3. Volatile loop accessing different TLS variables conditionally */
    for (selector = 0; selector < 6; selector++) {
        volatile int *target = NULL;
        
        switch (selector) {
            case 0: target = &tls_weak_hidden; break;
            case 1: target = &tls_public_default; break;
            case 2: target = &tls_common; break;
            case 3: target = &tls_protected; break;
            case 4: target = &tls_external; break;
            case 5: target = &tls_preserved; break;
        }
        
        if (target) {
            modify_tls_volatile(target);
        }
    }
    
    /* 4. Compute final checksum (prevents dead code elimination) */
    checksum = compute_tls_checksum();
    
    /* 5. Print checksum (prevents optimization) */
    printf("TLS checksum: %d\n", checksum);
    printf("tls_weak_hidden: %d\n", tls_weak_hidden);
    printf("tls_public_default: %d\n", tls_public_default);
    printf("tls_common: %d\n", tls_common);
    printf("tls_protected: %d\n", tls_protected);
    printf("tls_external: %d\n", tls_external);
    printf("tls_preserved: %d\n", tls_preserved);
    
    return checksum != 0 ? 0 : 1;
}
