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

/* 3. Common linkage TLS (tentative definition) */
__thread int tls_common __attribute__((common));

/* 4. External TLS declaration (defined in another TU would be) */
extern __thread int tls_external;

/* 5. Weak external TLS */
extern __thread int tls_weak_external __attribute__((weak));

/* 6. Protected visibility TLS */
__thread int tls_protected __attribute__((visibility("protected"))) = 200;

/* 7. DLL import simulation (Windows-style) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#else
/* On non-Windows, use dllimport attribute if supported */
#if __has_attribute(dllimport)
__thread int tls_dllimport __attribute__((dllimport));
#else
/* Fallback: just a regular TLS variable */
__thread int tls_dllimport = 300;
#endif
#endif

/* 8. Static TLS inside a function context */
static void function_with_static_tls(void) {
    static __thread int tls_function_static = 400;
    volatile int* volatile ptr = &tls_function_static;
    (void)ptr; /* Use pointer to prevent optimization */
}

/* 9. TLS with preserve attribute (via used) */
__thread int tls_preserved __attribute__((used)) = 500;

/* ===== HELPER FUNCTIONS TO FORCE TLS USAGE ===== */

NOINLINE static void use_tls_weak_hidden(volatile int* counter) {
    volatile int* ptr = &tls_weak_hidden;
    *counter += *ptr;
    tls_weak_hidden += 1;
}

NOINLINE static void use_tls_public_default(volatile int* counter) {
    volatile int* ptr = &tls_public_default;
    *counter += *ptr;
    tls_public_default += 2;
}

NOINLINE static void use_tls_common(volatile int* counter) {
    /* Force common TLS usage through multiple accesses */
    if (tls_common == 0) {
        tls_common = 600;
    }
    volatile int* ptr = &tls_common;
    *counter += *ptr;
    tls_common += 3;
}

NOINLINE static void use_tls_external(volatile int* counter) {
    /* External TLS - take address even if not defined */
    volatile int* ptr = &tls_external;
    *counter += (ptr != 0);
}

NOINLINE static void use_tls_protected(volatile int* counter) {
    volatile int* ptr = &tls_protected;
    *counter += *ptr;
    tls_protected += 4;
}

NOINLINE static void use_tls_dllimport(volatile int* counter) {
    volatile int* ptr = &tls_dllimport;
    *counter += (ptr != 0 ? *ptr : 0);
}

NOINLINE static void use_tls_preserved(volatile int* counter) {
    volatile int* ptr = &tls_preserved;
    *counter += *ptr;
    tls_preserved += 5;
}

/* ===== CONSTRUCTOR/DESTRUCTOR INTERACTION ===== */

CONSTRUCTOR static void init_tls_values(void) {
    /* Constructor accesses TLS before main */
    tls_public_default = 1234;
    tls_protected = 5678;
    
    /* Take addresses in constructor */
    volatile int* ptr1 = &tls_weak_hidden;
    volatile int* ptr2 = &tls_common;
    (void)ptr1;
    (void)ptr2;
}

DESTRUCTOR static void cleanup_tls_values(void) {
    /* Destructor also uses TLS */
    tls_public_default = 0;
}

/* ===== COMPLEX CONTROL FLOW WITH TLS ===== */

NOINLINE static int tls_conditional_access(volatile int selector) {
    volatile int result = 0;
    volatile int* tls_ptr = NULL;
    
    switch (selector % 5) {
        case 0:
            tls_ptr = &tls_weak_hidden;
            break;
        case 1:
            tls_ptr = &tls_public_default;
            break;
        case 2:
            tls_ptr = &tls_common;
            break;
        case 3:
            tls_ptr = &tls_protected;
            break;
        case 4:
            tls_ptr = &tls_preserved;
            break;
    }
    
    if (tls_ptr) {
        result = *tls_ptr;
        /* Modify through pointer */
        *(int*)tls_ptr += selector;
    }
    
    return result;
}

/* ===== MAIN FUNCTION WITH COMPREHENSIVE TLS USAGE ===== */

int main(void) {
    volatile int checksum = 0;
    volatile int i;
    
    /* 1. Call all TLS usage functions */
    use_tls_weak_hidden(&checksum);
    use_tls_public_default(&checksum);
    use_tls_common(&checksum);
    use_tls_external(&checksum);
    use_tls_protected(&checksum);
    use_tls_dllimport(&checksum);
    use_tls_preserved(&checksum);
    
    /* 2. Loop with volatile counter accessing TLS conditionally */
    for (i = 0; i < 10; i++) {
        volatile int selector = i;
        checksum += tls_conditional_access(selector);
        
        /* Direct volatile access to TLS in loop */
        volatile int* volatile ptr;
        if (i & 1) {
            ptr = &tls_weak_hidden;
        } else {
            ptr = &tls_public_default;
        }
        checksum += *ptr;
    }
    
    /* 3. Function with static TLS */
    function_with_static_tls();
    
    /* 4. Compute final checksum using all TLS variables */
    checksum += tls_weak_hidden;
    checksum += tls_public_default;
    checksum += tls_common;
    checksum += tls_protected;
    checksum += tls_preserved;
    
    /* 5. Take addresses of all TLS variables one more time */
    volatile int* addrs[] = {
        &tls_weak_hidden,
        &tls_public_default,
        &tls_common,
        &tls_external,
        &tls_protected,
        &tls_dllimport,
        &tls_preserved
    };
    
    for (i = 0; i < (int)(sizeof(addrs)/sizeof(addrs[0])); i++) {
        if (addrs[i]) {
            checksum += (uintptr_t)addrs[i] & 0xFF;
        }
    }
    
    printf("TLS checksum: %d\n", checksum);
    
    /* Return value based on TLS state */
    return (checksum > 1000) ? 0 : 1;
}

/* ===== C++ VERSION WITH NAMESPACES ===== */
#ifdef __cplusplus
namespace tls_test {
    thread_local int tls_namespace = 999;
    thread_local int tls_namespace_hidden __attribute__((visibility("hidden"))) = 888;
    
    class TLSUser {
    public:
        NOINLINE void use_namespace_tls(volatile int* counter) {
            volatile int* ptr1 = &tls_namespace;
            volatile int* ptr2 = &tls_namespace_hidden;
            *counter += *ptr1 + *ptr2;
            tls_namespace += 10;
            tls_namespace_hidden += 20;
        }
    };
}

/* C++ main that uses namespace TLS */
int cpp_main(void) {
    volatile int checksum = 0;
    tls_test::TLSUser user;
    
    user.use_namespace_tls(&checksum);
    
    /* Access namespace TLS directly */
    checksum += tls_test::tls_namespace;
    checksum += tls_test::tls_namespace_hidden;
    
    printf("C++ TLS checksum: %d\n", checksum);
    return 0;
}
#endif
