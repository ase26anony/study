/* tls_emulation_test.c - Test program for GCC emulated TLS attribute propagation */

/* Force emulated TLS even if platform supports native TLS */
#pragma GCC target("tls")

/* Prevent inlining to ensure TLS addresses are taken */
#define NOINLINE __attribute__((noinline))

/* Helper to prevent optimization */
static volatile int sink;

/* ========== TLS VARIABLES WITH DIVERSE ATTRIBUTES ========== */

/* 1. Weak TLS variable with hidden visibility */
__thread int tls_weak_hidden __attribute__((weak, visibility("hidden"))) = 42;

/* 2. Public TLS variable with default visibility */
__thread int tls_public_default = 100;

/* 3. Protected visibility TLS variable */
__thread int tls_protected __attribute__((visibility("protected"))) = 200;

/* 4. Common linkage (tentative definition) - no initializer */
__thread int tls_common __attribute__((common));

/* 5. External declaration - defined later */
extern __thread int tls_external;

/* 6. DLL import simulation (using weak as proxy) */
__thread int tls_dllimport __attribute__((weak, dllimport));

/* 7. Static TLS inside function context - tested in helper */
static __thread int tls_static_func = 300;

/* 8. TLS with preserve attribute (via used) */
__thread int tls_preserved __attribute__((used)) = 400;

/* ========== HELPER FUNCTIONS ========== */

NOINLINE static void use_tls_weak_hidden(void) {
    volatile int *p = &tls_weak_hidden;
    *p += 1;
    sink = *p;
}

NOINLINE static void use_tls_public_default(int x) {
    tls_public_default += x;
    volatile int *p = &tls_public_default;
    sink = *p;
}

NOINLINE static void use_tls_protected(void) {
    /* Take address and modify through pointer */
    int *ptr = &tls_protected;
    *ptr *= 2;
    sink = *ptr;
}

NOINLINE static void init_tls_common(void) {
    /* Force initialization of common TLS */
    if (tls_common == 0) {
        tls_common = 123;
    }
    sink = tls_common;
}

/* External TLS definition (matches extern declaration) */
__thread int tls_external = 500;

NOINLINE static void use_tls_external(void) {
    volatile int *p = &tls_external;
    *p -= 50;
    sink = *p;
}

NOINLINE static void use_tls_dllimport(void) {
    /* DLL imported TLS - may be weak */
    if (&tls_dllimport != NULL) {
        tls_dllimport = 600;
        sink = tls_dllimport;
    }
}

NOINLINE static void use_tls_static_func(void) {
    /* Static TLS inside function - different context */
    static __thread int local_tls = 700;
    volatile int *p = &local_tls;
    *p += 10;
    sink = *p;
    
    /* Also use the file-static one */
    tls_static_func++;
    sink += tls_static_func;
}

NOINLINE static void use_tls_preserved(void) {
    /* Preserved TLS should not be eliminated */
    tls_preserved = 800;
    sink = tls_preserved;
}

/* Constructor that interacts with TLS */
static void __attribute__((constructor)) init_tls_constructor(void) {
    /* Initialize TLS in constructor - tests DECL_PRESERVE_P */
    tls_public_default = 999;
    tls_protected = 888;
}

/* Function taking TLS addresses as parameters */
NOINLINE static int compute_checksum(
    int *a, int *b, int *c, int *d, 
    int *e, int *f, int *g, int *h) {
    return (*a + *b + *c + *d + *e + *f + *g + *h);
}

/* ========== MAIN EXECUTION FLOW ========== */

int main(void) {
    volatile int selector = 0;
    int checksum = 0;
    
    /* 1. Initialize all TLS variables through various functions */
    use_tls_weak_hidden();
    use_tls_public_default(5);
    use_tls_protected();
    init_tls_common();
    use_tls_external();
    use_tls_dllimport();
    use_tls_static_func();
    use_tls_preserved();
    
    /* 2. Conditional access based on volatile selector */
    for (selector = 0; selector < 8; selector++) {
        volatile int *ptr = NULL;
        
        switch (selector) {
            case 0: ptr = &tls_weak_hidden; break;
            case 1: ptr = &tls_public_default; break;
            case 2: ptr = &tls_protected; break;
            case 3: ptr = &tls_common; break;
            case 4: ptr = &tls_external; break;
            case 5: ptr = &tls_dllimport; break;
            case 6: ptr = &tls_static_func; break;
            case 7: ptr = &tls_preserved; break;
        }
        
        if (ptr) {
            *ptr += selector;
            sink = *ptr;
        }
    }
    
    /* 3. Compute checksum using addresses - forces all TLS to be live */
    checksum = compute_checksum(
        &tls_weak_hidden,
        &tls_public_default,
        &tls_protected,
        &tls_common,
        &tls_external,
        &tls_dllimport,
        &tls_static_func,
        &tls_preserved
    );
    
    /* 4. Print checksum to prevent elimination */
    printf("TLS checksum: %d\n", checksum);
    printf("tls_weak_hidden=%d\n", tls_weak_hidden);
    printf("tls_public_default=%d\n", tls_public_default);
    printf("tls_protected=%d\n", tls_protected);
    printf("tls_common=%d\n", tls_common);
    printf("tls_external=%d\n", tls_external);
    printf("tls_dllimport=%d\n", tls_dllimport);
    printf("tls_static_func=%d\n", tls_static_func);
    printf("tls_preserved=%d\n", tls_preserved);
    
    return checksum != 0 ? 0 : 1;
}
