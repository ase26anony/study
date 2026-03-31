/* tls_emulation_test.c - Test GCC's emulated TLS attribute propagation */
/* Compile with: gcc -O2 -femulated-tls -fno-common -fvisibility=hidden tls_emulation_test.c -o tls_test */

#include <stdio.h>
#include <stdint.h>

/* Prevent inlining to force TLS address usage */
#define NOINLINE __attribute__((noinline))

/* ===== TLS VARIABLES WITH DIVERSE ATTRIBUTES ===== */

/* 1. Weak TLS variable with hidden visibility */
__thread __attribute__((weak)) __attribute__((visibility("hidden"))) 
int tls_weak_hidden = 0x1234;

/* 2. Public TLS variable with default visibility */
__thread __attribute__((visibility("default")))
int tls_public_default = 0x5678;

/* 3. Protected visibility TLS variable */
__thread __attribute__((visibility("protected")))
volatile int tls_protected = 0x9ABC;

/* 4. Common linkage (tentative definition) - relies on -fno-common */
__thread int tls_common;

/* 5. Static TLS within function context - tests DECL_CONTEXT */
static void init_static_tls(void) {
    static __thread int tls_function_static = 0xDEF0;
    /* Use it to prevent optimization */
    volatile int *p = &tls_function_static;
    (void)p;
}

/* 6. External declaration (will be defined later) */
extern __thread int tls_external;

/* 7. DLL import simulation (using dllimport attribute if available) */
#ifdef _WIN32
    __declspec(dllimport) __thread int tls_dllimport;
#else
    /* On non-Windows, use weak to simulate similar behavior */
    __thread __attribute__((weak)) int tls_dllimport;
#endif

/* 8. Actual definition for external TLS variable */
__thread int tls_external = 0x1357;

/* ===== NOINLINE HELPER FUNCTIONS ===== */

NOINLINE static void use_tls_weak_hidden(int *out) {
    volatile int *volatile p = &tls_weak_hidden;
    *out += *p;
    tls_weak_hidden += 1;  /* Modify to ensure it's not const-folded */
}

NOINLINE static void use_tls_public_default(int *out) {
    volatile int *volatile p = &tls_public_default;
    *out += *p;
    tls_public_default ^= 0xFF;  /* Modify with XOR */
}

NOINLINE static void use_tls_protected(int *out) {
    /* Take address and use through volatile pointer */
    volatile int *volatile p = &tls_protected;
    *out += *p;
    *p += 1;  /* Direct modification through volatile pointer */
}

NOINLINE static void use_tls_common(int *out) {
    /* Multiple accesses to ensure usage marking */
    tls_common = tls_common + 1;
    volatile int *volatile p = &tls_common;
    *out += *p;
}

NOINLINE static void use_tls_external(int *out) {
    volatile int *volatile p = &tls_external;
    *out += *p;
    tls_external *= 2;  /* Modify */
}

/* ===== CONSTRUCTOR FUNCTION ===== */

__attribute__((constructor))
static void tls_constructor(void) {
    /* Initialize TLS variables in constructor */
    tls_common = 0x2468;
    tls_public_default = 0x1111;
    
    /* This should trigger DECL_PRESERVE_P propagation */
    volatile int *p = &tls_protected;
    *p = 0x2222;
}

/* ===== MAIN EXECUTION FLOW ===== */

int main(void) {
    volatile int selector = 0;
    int checksum = 0;
    
    /* Initialize function-static TLS */
    init_static_tls();
    
    /* Loop with volatile selector to prevent optimization */
    for (selector = 0; selector < 5; selector++) {
        switch (selector) {
            case 0:
                use_tls_weak_hidden(&checksum);
                break;
            case 1:
                use_tls_public_default(&checksum);
                break;
            case 2:
                use_tls_protected(&checksum);
                break;
            case 3:
                use_tls_common(&checksum);
                break;
            case 4:
                use_tls_external(&checksum);
                break;
        }
    }
    
    /* Additional conditional access based on checksum value */
    if (checksum & 1) {
        volatile int *volatile p = &tls_weak_hidden;
        checksum += *p;
    } else {
        volatile int *volatile p = &tls_public_default;
        checksum += *p;
    }
    
    /* Compute final checksum using all TLS variables */
    checksum += tls_weak_hidden;
    checksum += tls_public_default;
    checksum += tls_protected;
    checksum += tls_common;
    checksum += tls_external;
    
    printf("TLS checksum: 0x%08x\n", checksum);
    
    /* Runtime verification that we're using emulated TLS */
    printf("TLS variable addresses:\n");
    printf("  tls_weak_hidden:   %p\n", (void*)&tls_weak_hidden);
    printf("  tls_public_default:%p\n", (void*)&tls_public_default);
    printf("  tls_protected:     %p\n", (void*)&tls_protected);
    printf("  tls_common:        %p\n", (void*)&tls_common);
    printf("  tls_external:      %p\n", (void*)&tls_external);
    
    return checksum != 0 ? 0 : 1;
}
