/* tls_emulation_test.c - Test GCC's emulated TLS attribute propagation */
/* Compile with: gcc -O2 -femulated-tls -fno-common -fvisibility=hidden tls_emulation_test.c -o tls_test */

#include <stdio.h>
#include <stdint.h>

/* Prevent inlining to force TLS address usage */
#define NOINLINE __attribute__((noinline))
#define CONSTRUCTOR __attribute__((constructor))
#define DESTRUCTOR __attribute__((destructor))

/* ========== TLS VARIABLES WITH DIVERSE ATTRIBUTES ========== */

/* 1. Weak TLS variable with default visibility */
__thread int tls_weak_var __attribute__((weak)) = 42;

/* 2. Hidden visibility TLS variable */
__thread int tls_hidden_var __attribute__((visibility("hidden"))) = 100;

/* 3. Protected visibility TLS variable */
__thread int tls_protected_var __attribute__((visibility("protected"))) = 200;

/* 4. Common linkage (tentative definition) - requires -fno-common to test properly */
__thread int tls_common_var;  /* Tentative definition */

/* 5. External declaration (will be defined later) */
extern __thread int tls_external_var;

/* 6. DLL import simulation (using dllimport attribute if supported) */
#ifdef _WIN32
    __declspec(dllimport) __thread int tls_dllimport_var;
#else
    /* On non-Windows, use visibility to simulate similar behavior */
    __thread int tls_dllimport_var __attribute__((visibility("default")));
#endif

/* 7. Static TLS inside a function context */
static void func_with_static_tls(void) {
    static __thread int tls_func_static = 999;
    (void)tls_func_static;
}

/* 8. TLS with preserved flag (used in constructor) */
__thread int tls_preserved_var = 0xABCD;

/* 9. Public TLS variable */
__thread int tls_public_var = 333;

/* 10. Another weak variable with different initializer */
__thread int tls_weak2_var __attribute__((weak)) = 777;

/* External definition (for #5) */
__thread int tls_external_var = 555;

/* ========== HELPER FUNCTIONS FOR ADDRESS TAKING ========== */

NOINLINE static void use_tls_address_weak(volatile int** addr) {
    *addr = &tls_weak_var;
}

NOINLINE static void use_tls_address_hidden(volatile int** addr) {
    *addr = &tls_hidden_var;
}

NOINLINE static void use_tls_address_protected(volatile int** addr) {
    *addr = &tls_protected_var;
}

NOINLINE static void use_tls_address_common(volatile int** addr) {
    /* Force usage of common variable */
    tls_common_var = 123;
    *addr = &tls_common_var;
}

NOINLINE static void use_tls_address_external(volatile int** addr) {
    *addr = &tls_external_var;
}

NOINLINE static void use_tls_address_dllimport(volatile int** addr) {
    *addr = &tls_dllimport_var;
}

NOINLINE static void use_tls_address_preserved(volatile int** addr) {
    *addr = &tls_preserved_var;
}

NOINLINE static void use_tls_address_public(volatile int** addr) {
    *addr = &tls_public_var;
}

/* ========== CONSTRUCTOR/DESTRUCTOR INTERACTION ========== */

CONSTRUCTOR static void init_tls_vars(void) {
    /* This should mark DECL_PRESERVE_P on tls_preserved_var */
    tls_preserved_var = 0xDEADBEEF;
    
    /* Also use other TLS variables in constructor */
    tls_hidden_var += 1;
    tls_protected_var *= 2;
}

DESTRUCTOR static void cleanup_tls_vars(void) {
    /* Access TLS in destructor */
    volatile int dummy = tls_preserved_var;
    (void)dummy;
}

/* ========== VOLATILE ACCESS PATTERNS ========== */

NOINLINE static uint32_t compute_tls_checksum(void) {
    volatile int* addrs[8];
    uint32_t checksum = 0;
    volatile int selector = 0;
    
    /* Initialize address array through volatile pointers */
    use_tls_address_weak(&addrs[0]);
    use_tls_address_hidden(&addrs[1]);
    use_tls_address_protected(&addrs[2]);
    use_tls_address_common(&addrs[3]);
    use_tls_address_external(&addrs[4]);
    use_tls_address_dllimport(&addrs[5]);
    use_tls_address_preserved(&addrs[6]);
    use_tls_address_public(&addrs[7]);
    
    /* Volatile loop accessing different TLS variables */
    for (volatile int i = 0; i < 100; i++) {
        selector = i % 8;
        
        /* Force actual memory access through volatile pointer */
        volatile int* ptr = addrs[selector];
        checksum += *ptr;
        
        /* Modify some TLS variables */
        if (selector == 0) {
            *ptr += 1;  /* Modify weak var */
        } else if (selector == 3) {
            *ptr ^= 0xFF;  /* Modify common var */
        }
    }
    
    return checksum;
}

/* ========== BLOCK SCOPE TLS TEST ========== */

NOINLINE static void test_block_scope_tls(void) {
    /* TLS in block scope */
    static __thread int tls_block_static = 888;
    __thread int tls_block_auto;
    
    volatile int* ptr1 = &tls_block_static;
    volatile int* ptr2 = &tls_block_auto;
    
    *ptr1 += 1;
    *ptr2 = *ptr1 + 10;
    
    /* Use the static TLS from function context */
    func_with_static_tls();
}

/* ========== C++ SPECIFIC TESTS (if compiled as C++) ========== */

#ifdef __cplusplus
namespace tls_namespace {
    __thread int tls_in_namespace = 1234;
    
    class TLSUser {
    public:
        NOINLINE void use_namespace_tls() {
            volatile int* ptr = &tls_in_namespace;
            *ptr += 100;
        }
        
        static __thread int tls_class_member;
    };
    
    __thread int TLSUser::tls_class_member = 4321;
}
#endif

/* ========== MAIN FUNCTION ========== */

int main(void) {
    uint32_t checksum;
    
    printf("Testing emulated TLS attribute propagation...\n");
    
    /* 1. Force usage of all TLS variables */
    test_block_scope_tls();
    
    /* 2. Compute checksum through volatile accesses */
    checksum = compute_tls_checksum();
    
    /* 3. Additional conditional TLS usage */
    volatile int mode = 0;
    for (int i = 0; i < 10; i++) {
        mode = i % 3;
        
        if (mode == 0) {
            volatile int* ptr = &tls_weak_var;
            *ptr += i;
        } else if (mode == 1) {
            volatile int* ptr = &tls_hidden_var;
            *ptr -= i;
        } else {
            volatile int* ptr = &tls_protected_var;
            *ptr ^= i;
        }
    }
    
#ifdef __cplusplus
    /* C++ specific tests */
    tls_namespace::TLSUser user;
    user.use_namespace_tls();
    tls_namespace::TLSUser::tls_class_member += 1;
#endif
    
    /* 4. Print checksum to prevent elimination */
    printf("TLS checksum: 0x%08X\n", checksum);
    
    /* 5. Verify emulated TLS structure by checking addresses */
    printf("TLS variable addresses:\n");
    printf("  weak:        %p\n", (void*)&tls_weak_var);
    printf("  hidden:      %p\n", (void*)&tls_hidden_var);
    printf("  protected:   %p\n", (void*)&tls_protected_var);
    printf("  common:      %p\n", (void*)&tls_common_var);
    printf("  external:    %p\n", (void*)&tls_external_var);
    printf("  preserved:   %p\n", (void*)&tls_preserved_var);
    
    return 0;
}
