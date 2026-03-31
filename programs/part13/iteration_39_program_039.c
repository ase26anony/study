/* tls_emulation_test.c - Test program for GCC emulated TLS attribute propagation */

/* Force emulated TLS even if native TLS is available */
#pragma GCC target("tls")

/* Disable inlining to ensure TLS addresses are taken */
#define NOINLINE __attribute__((noinline))

/* Helper to prevent optimization */
static volatile int sink;

/* ========== TLS VARIABLES WITH DIVERSE ATTRIBUTES ========== */

/* 1. Weak TLS variable with hidden visibility */
__thread int tls_weak_hidden __attribute__((weak, visibility("hidden"))) = 42;

/* 2. Public TLS variable with default visibility */
__thread int tls_public_default = 100;

/* 3. Protected visibility TLS */
__thread int tls_protected __attribute__((visibility("protected"))) = 200;

/* 4. Common linkage (tentative definition) - no initializer */
__thread int tls_common;

/* 5. External declaration (defined in another TU) */
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
    sink = tls_func_static;
}

/* 8. TLS with constructor priority */
__thread int tls_with_ctor __attribute__((used)) = 400;

/* ========== NOINLINE FUNCTIONS FOR ADDRESS TAKING ========== */

NOINLINE void use_tls_weak_hidden(int *out) {
    volatile int *volatile ptr = &tls_weak_hidden;
    *out += *ptr;
    tls_weak_hidden++;
}

NOINLINE void use_tls_public_default(int *out) {
    /* Take address and use in computation */
    int *addr = &tls_public_default;
    *out ^= *addr;
    tls_public_default *= 2;
}

NOINLINE void use_tls_protected(int *out) {
    /* Complex enough to prevent optimization */
    for (volatile int i = 0; i < 3; i++) {
        *out += tls_protected + i;
    }
    tls_protected--;
}

NOINLINE void use_tls_common(int *out) {
    /* Initialize if zero */
    if (tls_common == 0) {
        tls_common = 12345;
    }
    *out = tls_common % 1000;
}

NOINLINE void use_tls_dllimport(int *out) {
    /* Simulate DLL import usage pattern */
    volatile int *ptr = &tls_dllimport;
    if (*ptr != 0) {
        *out += *ptr;
    } else {
        *out -= 1;
    }
}

/* ========== CONSTRUCTOR FUNCTION ========== */

__attribute__((constructor(101)))
static void init_tls_in_constructor(void) {
    /* This should mark DECL_PRESERVE_P */
    tls_with_ctor = 999;
    
    /* Also touch other TLS variables */
    tls_common = 555;
    sink = tls_public_default;
}

/* ========== CONDITIONAL ACCESS PATTERNS ========== */

NOINLINE int conditional_tls_access(volatile int selector) {
    int result = 0;
    
    /* Switch ensures multiple control paths */
    switch (selector & 7) {
        case 0:
            result = tls_weak_hidden;
            break;
        case 1:
            result = tls_public_default;
            break;
        case 2:
            result = tls_protected;
            break;
        case 3:
            result = tls_common;
            break;
        case 4:
            result = tls_with_ctor;
            break;
        case 5:
            /* Take address in one path */
            result = *(int*)&tls_dllimport;
            break;
        case 6:
            /* Multiple TLS vars in one case */
            result = tls_weak_hidden + tls_public_default;
            break;
        case 7:
            /* Complex expression */
            result = tls_protected * tls_common;
            break;
    }
    
    return result;
}

/* ========== MAIN EXECUTION FLOW ========== */

int main(void) {
    int checksum = 0;
    volatile int i;
    
    /* 1. Initialize external TLS (simulate definition) */
    __thread int tls_external = 777;
    
    /* 2. Call noinline functions that take TLS addresses */
    use_tls_weak_hidden(&checksum);
    use_tls_public_default(&checksum);
    use_tls_protected(&checksum);
    use_tls_common(&checksum);
    use_tls_dllimport(&checksum);
    
    /* 3. Volatile loop with conditional TLS access */
    for (i = 0; i < 100; i++) {
        checksum ^= conditional_tls_access(i);
        
        /* Volatile access pattern */
        volatile int *volatile ptr;
        if (i & 1) {
            ptr = &tls_weak_hidden;
        } else {
            ptr = &tls_public_default;
        }
        checksum += *ptr;
    }
    
    /* 4. Function with static TLS */
    func_with_static_tls();
    
    /* 5. Compute final checksum using all TLS variables */
    checksum += tls_weak_hidden;
    checksum += tls_public_default;
    checksum += tls_protected;
    checksum += tls_common;
    checksum += tls_external;
    checksum += tls_dllimport;
    checksum += tls_with_ctor;
    
    /* 6. Prevent dead code elimination */
    sink = checksum;
    
    /* Return value depends on TLS state */
    return (checksum & 255);
}
