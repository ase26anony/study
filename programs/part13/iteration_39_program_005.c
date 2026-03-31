/* tls_emulation_test.c - Test GCC's emulated TLS attribute propagation */
/* Compile with: gcc -O2 -femulated-tls -fno-common -fvisibility=hidden tls_emulation_test.c -o tls_test */

#include <stdio.h>
#include <stdint.h>

/* Prevent inlining to force TLS address usage */
#define NOINLINE __attribute__((noinline))
#define CONSTRUCTOR __attribute__((constructor))
#define DESTRUCTOR __attribute__((destructor))

/* ========== TLS VARIABLES WITH DIVERSE ATTRIBUTES ========== */

/* 1. Weak TLS variable with hidden visibility */
__thread __attribute__((weak, visibility("hidden"))) 
int tls_weak_hidden = 0x1234;

/* 2. Public TLS variable with default visibility */
__thread int tls_public_default = 0x5678;

/* 3. Protected visibility TLS variable */
__thread __attribute__((visibility("protected"))) 
int tls_protected = 0x9ABC;

/* 4. Common linkage TLS (tentative definition) */
__thread int tls_common;

/* 5. External TLS declaration (defined in same file) */
extern __thread int tls_external;
__thread int tls_external = 0xDEF0;

/* 6. DLL import style attribute (simulated) */
#ifdef _WIN32
__thread __declspec(dllimport) int tls_dllimport;
#else
__thread __attribute__((dllimport)) int tls_dllimport;
#endif

/* 7. Static TLS inside a function context */
static void function_with_static_tls(void) {
    static __thread int tls_function_static = 0x1111;
    volatile int* volatile ptr = &tls_function_static;
    *ptr += 1;
}

/* 8. TLS with used attribute to ensure TREE_USED is set */
__thread __attribute__((used)) int tls_used_attr = 0x2222;

/* ========== HELPER FUNCTIONS FOR ADDRESS TAKING ========== */

NOINLINE static void use_tls_weak_hidden(volatile int** out) {
    *out = &tls_weak_hidden;
}

NOINLINE static void use_tls_public_default(volatile int** out) {
    *out = &tls_public_default;
}

NOINLINE static void use_tls_protected(volatile int** out) {
    *out = &tls_protected;
}

NOINLINE static void use_tls_common(volatile int** out) {
    *out = &tls_common;
}

NOINLINE static void use_tls_external(volatile int** out) {
    *out = &tls_external;
}

NOINLINE static void use_tls_used_attr(volatile int** out) {
    *out = &tls_used_attr;
}

/* Function that takes multiple TLS addresses */
NOINLINE static uintptr_t mix_tls_addresses(void) {
    uintptr_t hash = 0;
    volatile int* ptr;
    
    use_tls_weak_hidden(&ptr);
    hash ^= (uintptr_t)ptr;
    
    use_tls_public_default(&ptr);
    hash ^= (uintptr_t)ptr >> 4;
    
    use_tls_protected(&ptr);
    hash ^= (uintptr_t)ptr << 2;
    
    return hash;
}

/* ========== CONSTRUCTOR/DESTRUCTOR INTERACTION ========== */

CONSTRUCTOR static void init_tls_in_constructor(void) {
    /* This should trigger DECL_PRESERVE_P propagation */
    tls_public_default = 0xCAFE;
    tls_protected = 0xBEEF;
    
    /* Access through volatile pointer */
    volatile int* volatile ptr = &tls_used_attr;
    *ptr = 0xDEAD;
}

DESTRUCTOR static void cleanup_tls_in_destructor(void) {
    /* Final access to ensure preservation */
    volatile int* volatile ptr = &tls_public_default;
    *ptr = 0;
}

/* ========== COMPLEX CONTROL FLOW WITH VOLATILE ========== */

NOINLINE static uint32_t access_tls_based_on_volatile(volatile int selector) {
    volatile int* tls_ptr = NULL;
    
    switch (selector % 6) {
        case 0: tls_ptr = &tls_weak_hidden; break;
        case 1: tls_ptr = &tls_public_default; break;
        case 2: tls_ptr = &tls_protected; break;
        case 3: tls_ptr = &tls_common; break;
        case 4: tls_ptr = &tls_external; break;
        case 5: tls_ptr = &tls_used_attr; break;
    }
    
    if (tls_ptr) {
        /* Volatile access pattern prevents optimization */
        int val = *tls_ptr;
        *tls_ptr = val + selector;
        return (uint32_t)(uintptr_t)tls_ptr ^ val;
    }
    
    return 0;
}

/* ========== C++ STYLE NAMESPACE EMULATION ========== */

#ifdef __cplusplus
namespace tls_namespace {
    __thread int tls_in_namespace = 0x8888;
    
    class TLSUser {
    public:
        NOINLINE void modify_tls() {
            volatile int* ptr = &tls_in_namespace;
            *ptr += 1;
        }
    };
}
#else
/* C version - use struct to simulate context */
struct TLSContext {
    __thread int tls_in_struct;
};

static __thread int tls_in_struct_global = 0x8888;
#endif

/* ========== MAIN EXECUTION FLOW ========== */

int main(void) {
    volatile int counter = 0;
    uint32_t checksum = 0;
    
    /* 1. Force usage of all TLS variables */
    function_with_static_tls();
    
    /* 2. Mix addresses - ensures TREE_USED is set */
    checksum ^= mix_tls_addresses();
    
    /* 3. Volatile loop with conditional TLS access */
    for (volatile int i = 0; i < 100; i++) {
        checksum += access_tls_based_on_volatile(i);
        counter = i; /* Prevent loop optimization */
    }
    
    /* 4. Direct volatile pointer manipulation */
    volatile int* volatile ptrs[6];
    ptrs[0] = &tls_weak_hidden;
    ptrs[1] = &tls_public_default;
    ptrs[2] = &tls_protected;
    ptrs[3] = &tls_common;
    ptrs[4] = &tls_external;
    ptrs[5] = &tls_used_attr;
    
    for (int j = 0; j < 6; j++) {
        if (ptrs[j]) {
            *ptrs[j] += j;
            checksum ^= (uint32_t)*ptrs[j];
        }
    }
    
    /* 5. C++ style usage if compiled as C++ */
#ifdef __cplusplus
    tls_namespace::TLSUser user;
    user.modify_tls();
    checksum ^= tls_namespace::tls_in_namespace;
#else
    /* C version */
    volatile int* struct_ptr = &tls_in_struct_global;
    *struct_ptr += 1;
    checksum ^= *struct_ptr;
#endif
    
    /* 6. Compute final checksum to prevent elimination */
    printf("TLS checksum: 0x%08X\n", checksum);
    printf("Counter (volatile): %d\n", counter);
    
    /* 7. Verify emulated TLS is being used */
    printf("TLS variable addresses:\n");
    printf("  tls_weak_hidden:   %p\n", (void*)&tls_weak_hidden);
    printf("  tls_public_default:%p\n", (void*)&tls_public_default);
    printf("  tls_protected:     %p\n", (void*)&tls_protected);
    
    return (checksum != 0) ? 0 : 1;
}

/* ========== ADDITIONAL EXTERNAL REFERENCES ========== */

/* External reference to force external attribute */
extern __thread int tls_purely_external;

NOINLINE static void reference_external_tls(void) {
    /* Just taking address, no definition in this file */
    volatile int* volatile ptr = &tls_purely_external;
    (void)ptr;
}

/* Force a call from constructor */
CONSTRUCTOR static void init_references(void) {
    reference_external_tls();
}
