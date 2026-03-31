/* tls_emulation_test.c - Test program for GCC emulated TLS attribute propagation */

/* Force emulated TLS even on platforms with native support */
#pragma GCC target("tls")

/* Prevent inlining to ensure TLS addresses are taken */
#define NOINLINE __attribute__((noinline))
#define CONSTRUCTOR __attribute__((constructor))
#define DESTRUCTOR __attribute__((destructor))

/* ===== TLS VARIABLES WITH DIVERSE ATTRIBUTES ===== */

/* 1. Weak TLS variable with hidden visibility */
__thread __attribute__((weak, visibility("hidden"))) 
int tls_weak_hidden = 42;

/* 2. Public TLS variable with default visibility */
__thread int tls_public_default = 100;

/* 3. Common linkage TLS (tentative definition) */
__thread int tls_common __attribute__((common));

/* 4. Protected visibility TLS */
__thread __attribute__((visibility("protected"))) 
int tls_protected = 200;

/* 5. DLL import simulation (using visibility for Unix-like) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#else
__thread __attribute__((visibility("default"), weak)) 
int tls_dllimport;
#endif

/* 6. Static TLS inside a function context */
static void function_with_static_tls(void) {
    static __thread int tls_function_static = 300;
    volatile int* volatile ptr = &tls_function_static;
    (void)ptr;
}

/* 7. External declaration (defined later) */
extern __thread int tls_external;

/* 8. TLS with preserve attribute (via used) */
__thread __attribute__((used)) int tls_preserved = 400;

/* 9. Weak external TLS */
extern __thread __attribute__((weak)) int tls_weak_external;

/* ===== HELPER FUNCTIONS FOR ADDRESS TAKING ===== */

NOINLINE static void use_tls_weak_hidden(volatile int* out) {
    volatile int* ptr = &tls_weak_hidden;
    *out += *ptr;
    tls_weak_hidden++;
}

NOINLINE static void use_tls_public_default(volatile int* out) {
    volatile int* ptr = &tls_public_default;
    *out += *ptr;
    tls_public_default--;
}

NOINLINE static void use_tls_common(volatile int* out) {
    volatile int* ptr = &tls_common;
    *out += *ptr;
    tls_common += 2;
}

NOINLINE static void use_tls_protected(volatile int* out) {
    volatile int* ptr = &tls_protected;
    *out += *ptr;
    tls_protected *= 2;
}

NOINLINE static void use_tls_dllimport(volatile int* out) {
    volatile int* ptr = &tls_dllimport;
    *out += *ptr ? *ptr : 1;
}

NOINLINE static void use_tls_external(volatile int* out) {
    volatile int* ptr = &tls_external;
    *out += *ptr;
}

NOINLINE static void use_tls_preserved(volatile int* out) {
    volatile int* ptr = &tls_preserved;
    *out += *ptr;
    tls_preserved += 100;
}

NOINLINE static void use_tls_weak_external(volatile int* out) {
    if (&tls_weak_external != NULL) {
        volatile int* ptr = &tls_weak_external;
        *out += *ptr;
    }
}

/* ===== CONSTRUCTOR/DESTRUCTOR INTERACTIONS ===== */

CONSTRUCTOR static void init_tls_values(void) {
    /* This constructor runs before main, testing DECL_PRESERVE_P */
    tls_common = 999;
    tls_public_default = 1234;
    
    /* Take address in constructor to ensure TREE_USED is set */
    volatile int* volatile ptr1 = &tls_weak_hidden;
    volatile int* volatile ptr2 = &tls_protected;
    (void)ptr1;
    (void)ptr2;
}

DESTRUCTOR static void cleanup_tls_values(void) {
    /* Ensure TLS variables are used until the end */
    volatile int dummy = tls_public_default + tls_preserved;
    (void)dummy;
}

/* ===== EXTERNAL TLS DEFINITION ===== */
__thread int tls_external = 555;

/* ===== MAIN FUNCTION WITH COMPLEX CONTROL FLOW ===== */

int main(void) {
    volatile int checksum = 0;
    volatile int selector = 0;
    
    /* Force TREE_USED on all TLS variables by taking addresses */
    function_with_static_tls();
    
    /* Complex control flow with volatile selector */
    for (volatile int i = 0; i < 10; i++) {
        selector = i % 7;
        
        switch (selector) {
            case 0:
                use_tls_weak_hidden(&checksum);
                break;
            case 1:
                use_tls_public_default(&checksum);
                break;
            case 2:
                use_tls_common(&checksum);
                break;
            case 3:
                use_tls_protected(&checksum);
                break;
            case 4:
                use_tls_dllimport(&checksum);
                break;
            case 5:
                use_tls_external(&checksum);
                break;
            case 6:
                use_tls_preserved(&checksum);
                break;
        }
        
        /* Mix in weak external check */
        if (i % 3 == 0) {
            use_tls_weak_external(&checksum);
        }
    }
    
    /* Final computation using all TLS variables */
    checksum += tls_weak_hidden;
    checksum += tls_public_default;
    checksum += tls_common;
    checksum += tls_protected;
    checksum += tls_dllimport ? tls_dllimport : 1;
    checksum += tls_external;
    checksum += tls_preserved;
    
    /* Prevent dead code elimination */
    volatile int result = checksum;
    
    /* Simple output to prevent optimization */
    if (result != 0) {
        return 0;  /* Success */
    }
    
    return 1;
}
