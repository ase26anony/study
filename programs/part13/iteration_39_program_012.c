/* tls_emulation_test.c - Test GCC's emulated TLS attribute propagation */
/* Compile with: gcc -O2 -femulated-tls -fno-common -fvisibility=hidden tls_emulation_test.c -o tls_test */

#include <stdio.h>
#include <stdint.h>

/* Prevent inlining to force TLS address usage */
#define NOINLINE __attribute__((noinline))

/* ===== TLS VARIABLES WITH DIVERSE ATTRIBUTES ===== */

/* 1. Weak TLS variable with hidden visibility */
__thread __attribute__((weak, visibility("hidden"))) 
int tls_weak_hidden = 0x1234;

/* 2. Public TLS variable with default visibility */
__thread int tls_public_default = 0x5678;

/* 3. Protected visibility TLS variable */
__thread __attribute__((visibility("protected"))) 
int tls_protected = 0x9ABC;

/* 4. Common linkage (tentative definition) - requires -fno-common */
__thread int tls_common __attribute__((common));

/* 5. External declaration (defined in same file later) */
extern __thread int tls_external;

/* 6. DLL import simulation (using dllimport attribute if supported) */
#ifdef _WIN32
    __declspec(dllimport) __thread int tls_dllimport;
#else
    /* On non-Windows, use weak to simulate similar behavior */
    __thread __attribute__((weak)) int tls_dllimport;
#endif

/* 7. Static TLS inside a function context */
static void func_with_static_tls(void) {
    static __thread int tls_func_static = 0xDEF0;
    (void)tls_func_static; /* Reference to prevent optimization */
}

/* 8. TLS with constructor initialization */
__thread int tls_with_constructor = 0;

/* ===== FUNCTION DECLARATIONS ===== */
NOINLINE static void use_tls_address(int *addr);
NOINLINE static void modify_tls_via_volatile(volatile int *addr);
NOINLINE static int compute_tls_checksum(void);

/* ===== TLS VARIABLE DEFINITIONS ===== */
/* Define the external TLS variable */
__thread int tls_external = 0x2468;

/* ===== CONSTRUCTOR FUNCTION ===== */
/* Tests DECL_PRESERVE_P propagation */
static void __attribute__((constructor)) tls_constructor(void) {
    tls_with_constructor = 0xCAFE;
    tls_public_default++; /* Modify another TLS variable */
}

/* ===== HELPER FUNCTIONS ===== */
NOINLINE static void use_tls_address(int *addr) {
    /* Force TLS variable to be marked as used */
    static volatile int sink;
    sink = *addr;
    (void)sink;
}

NOINLINE static void modify_tls_via_volatile(volatile int *addr) {
    /* Volatile access prevents optimization */
    *addr = *addr + 1;
}

NOINLINE static int compute_tls_checksum(void) {
    /* Compute checksum of all TLS variables to ensure they're used */
    int sum = 0;
    
    /* Take addresses to force TLS structure generation */
    int *addrs[] = {
        &tls_weak_hidden,
        &tls_public_default,
        &tls_protected,
        &tls_common,
        &tls_external,
        &tls_with_constructor,
        NULL
    };
    
    for (int i = 0; addrs[i] != NULL; i++) {
        sum = (sum * 31) + *addrs[i];
    }
    
    return sum;
}

/* ===== MAIN FUNCTION WITH COMPLEX CONTROL FLOW ===== */
int main(void) {
    volatile int selector = 0;
    int checksum = 0;
    
    /* Initialize common TLS variable */
    tls_common = 0x1357;
    
    /* Call function with static TLS to ensure its context is set */
    func_with_static_tls();
    
    /* Loop with volatile selector to access different TLS variables */
    for (selector = 0; selector < 6; selector++) {
        volatile int *target = NULL;
        
        switch (selector) {
            case 0: target = &tls_weak_hidden; break;
            case 1: target = &tls_public_default; break;
            case 2: target = &tls_protected; break;
            case 3: target = &tls_common; break;
            case 4: target = &tls_external; break;
            case 5: target = &tls_with_constructor; break;
        }
        
        if (target) {
            /* Use both access methods */
            use_tls_address((int*)target);
            modify_tls_via_volatile(target);
        }
    }
    
    /* Compute final checksum */
    checksum = compute_tls_checksum();
    
    /* Print checksum to prevent dead code elimination */
    printf("TLS checksum: 0x%08X\n", checksum);
    
    /* Verify emulated TLS is being used */
    #ifdef __EMUTLS__
        printf("Using emulated TLS\n");
    #endif
    
    return (checksum != 0) ? 0 : 1;
}

/* ===== ADDITIONAL USAGE IN SEPARATE COMPILATION UNIT SIMULATION ===== */
/* This would normally be in a separate file, but included here for completeness */
NOINLINE void additional_tls_usage(void) {
    /* Reference DLL import style TLS */
    #ifdef _WIN32
        extern __declspec(dllimport) __thread int tls_dllimport;
    #else
        extern __thread __attribute__((weak)) int tls_dllimport;
    #endif
    
    if (&tls_dllimport != NULL) {
        tls_dllimport = 0xBEAD;
    }
}
