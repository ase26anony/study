/* tls_emulation_test.c - Test GCC's emulated TLS attribute propagation */

/* Force emulated TLS even if native is available */
#pragma GCC target("tls-models=emulated")

/* Prevent inlining to ensure TLS addresses are taken */
#define NOINLINE __attribute__((noinline))
#define CONSTRUCTOR __attribute__((constructor))
#define DESTRUCTOR __attribute__((destructor))

/* ===== TLS VARIABLES WITH DIVERSE ATTRIBUTES ===== */

/* 1. Weak TLS variable with default visibility */
__thread int tls_weak_var __attribute__((weak)) = 42;

/* 2. Hidden visibility TLS */
__thread int tls_hidden_var __attribute__((visibility("hidden"))) = 100;

/* 3. Protected visibility TLS */
__thread int tls_protected_var __attribute__((visibility("protected"))) = 200;

/* 4. Common linkage (tentative definition) - no initializer */
__thread int tls_common_var;

/* 5. External declaration (defined in same file later) */
extern __thread int tls_external_var;

/* 6. DLL import style (simulated with weak) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport_var;
#else
__thread int tls_dllimport_var __attribute__((weak));
#endif

/* 7. Static TLS inside a function context */
static void func_with_static_tls(void) {
    static __thread int tls_func_static = 999;
    (void)tls_func_static;
}

/* 8. TLS with preserved attribute (via used) */
__thread int tls_used_var __attribute__((used)) = 333;

/* 9. Public TLS variable */
__thread int tls_public_var = 444;

/* 10. Definition of the external TLS variable */
__thread int tls_external_var = 555;

/* ===== NOINLINE HELPER FUNCTIONS ===== */

NOINLINE void use_tls_weak(int *out) {
    volatile int *volatile ptr = &tls_weak_var;
    *out += *ptr;
    tls_weak_var += 1;  /* Modify to ensure it's not const-folded */
}

NOINLINE void use_tls_hidden(int *out) {
    volatile int *volatile ptr = &tls_hidden_var;
    *out += *ptr;
    tls_hidden_var ^= 0xAAAA;  /* Non-linear modification */
}

NOINLINE void use_tls_protected(int *out) {
    /* Take address in a way that can't be optimized away */
    int *ptr = &tls_protected_var;
    volatile int *vptr = ptr;
    *out += *vptr;
    tls_protected_var *= 2;
}

NOINLINE void use_tls_common(int *out) {
    /* Force common TLS to be allocated */
    static volatile int sink;
    tls_common_var = *out + 1;
    sink = tls_common_var;
    *out = tls_common_var;
}

NOINLINE void use_tls_external(int *out) {
    volatile int *volatile ptr = &tls_external_var;
    *out += *ptr;
    tls_external_var -= 3;
}

NOINLINE void use_tls_dllimport(int *out) {
    /* Simulate DLL import usage pattern */
    volatile int *volatile ptr = &tls_dllimport_var;
    if (*ptr == 0) {
        tls_dllimport_var = 1234;
    }
    *out += tls_dllimport_var;
}

NOINLINE void use_tls_public(int *out) {
    /* Complex access pattern to prevent optimization */
    for (volatile int i = 0; i < 3; i++) {
        *out += tls_public_var + i;
    }
    tls_public_var++;
}

/* ===== CONSTRUCTOR/DESTRUCTOR INTERACTIONS ===== */

CONSTRUCTOR static void init_tls_values(void) {
    /* Constructor accesses multiple TLS variables */
    tls_hidden_var = 0xDEAD;
    tls_protected_var = 0xBEEF;
    tls_external_var = 0xCAFE;
    
    /* Take address to force TREE_USED */
    volatile int *volatile addr = &tls_used_var;
    (void)addr;
}

DESTRUCTOR static void cleanup_tls(void) {
    /* Destructor also uses TLS */
    volatile int sink = tls_public_var;
    (void)sink;
}

/* ===== VOLATILE CONTROL FLOW ===== */

NOINLINE int volatile_tls_selector(int selector) {
    volatile int result = 0;
    volatile int *volatile ptrs[] = {
        &tls_weak_var,
        &tls_hidden_var,
        &tls_protected_var,
        &tls_common_var,
        &tls_external_var,
        &tls_dllimport_var,
        &tls_public_var,
        &tls_used_var
    };
    
    /* Volatile loop prevents optimization */
    for (volatile int i = 0; i < selector; i++) {
        if (i < (int)(sizeof(ptrs)/sizeof(ptrs[0]))) {
            result += *ptrs[i % 8];
        }
    }
    
    return result;
}

/* ===== MAIN EXECUTION FLOW ===== */

int main(void) {
    int checksum = 0;
    volatile int selector = 8; /* Force all paths */
    
    /* 1. Initialize common TLS variable */
    tls_common_var = 777;
    
    /* 2. Call all helper functions */
    use_tls_weak(&checksum);
    use_tls_hidden(&checksum);
    use_tls_protected(&checksum);
    use_tls_common(&checksum);
    use_tls_external(&checksum);
    use_tls_dllimport(&checksum);
    use_tls_public(&checksum);
    
    /* 3. Call function with static TLS */
    func_with_static_tls();
    
    /* 4. Volatile selector pattern */
    checksum += volatile_tls_selector(selector);
    
    /* 5. Direct volatile accesses */
    volatile int *volatile vptr1 = &tls_weak_var;
    volatile int *volatile vptr2 = &tls_hidden_var;
    checksum += *vptr1 + *vptr2;
    
    /* 6. Conditional based on TLS values */
    if (tls_external_var > 0) {
        checksum += tls_protected_var;
    } else {
        checksum += tls_public_var;
    }
    
    /* 7. Loop with volatile counter accessing TLS */
    for (volatile int i = 0; i < 2; i++) {
        checksum += (i == 0) ? tls_weak_var : tls_hidden_var;
    }
    
    /* Final checksum output prevents elimination */
    volatile int final_result = checksum;
    
    /* Use all TLS variables one more time */
    return final_result 
           + tls_weak_var + tls_hidden_var + tls_protected_var
           + tls_common_var + tls_external_var + tls_dllimport_var
           + tls_public_var + tls_used_var;
}
