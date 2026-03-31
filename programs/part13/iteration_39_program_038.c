/* tls_emulation_test.c - Test GCC's emulated TLS attribute propagation */

/* Force emulated TLS even if native is available */
#pragma GCC target("tls")  /* Some targets interpret this as "use emulated TLS" */

/* For DLL attributes on Windows-like targets */
#ifdef _WIN32
#define DLL_IMPORT __declspec(dllimport)
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_IMPORT __attribute__((dllimport))
#define DLL_EXPORT __attribute__((dllexport))
#endif

/* Prevent inlining to force TLS address usage */
#define NOINLINE __attribute__((noinline))

/* Global TLS variables with various attributes */

/* 1. Weak TLS with hidden visibility */
__thread int tls_weak_hidden __attribute__((weak, visibility("hidden"))) = 42;

/* 2. Public TLS with default visibility (external linkage) */
__thread int tls_public_default = 100;

/* 3. Protected visibility TLS */
__thread int tls_protected __attribute__((visibility("protected"))) = 200;

/* 4. Common linkage TLS (tentative definition) */
__thread int tls_common;

/* 5. Static TLS (internal linkage) with constructor usage */
static __thread int tls_static_internal = 300;

/* 6. DLL imported TLS (simulated) */
extern __thread int tls_dll_imported DLL_IMPORT;

/* 7. Weak external TLS declaration */
extern __thread int tls_weak_external __attribute__((weak));

/* 8. TLS in different context - inside a struct-like context */
struct Container {
    /* This will test DECL_CONTEXT copying */
    static __thread int tls_in_struct;
};
__thread int Container::tls_in_struct = 400;

/* Forward declarations for helper functions */
NOINLINE void use_tls_weak_hidden(int *out);
NOINLINE void use_tls_public_default(int *out);
NOINLINE void use_tls_protected(int *out);
NOINLINE void use_tls_common(int *out);
NOINLINE void use_tls_static_internal(int *out);
NOINLINE void use_tls_addresses(void **ptrs, int count);
NOINLINE int compute_checksum(void);

/* Volatile counter to prevent optimization */
static volatile int volatile_counter = 0;

/* Constructor that accesses TLS */
__attribute__((constructor(101)))
static void init_tls_in_constructor(void) {
    /* This should mark DECL_PRESERVE_P on the TLS vars we use */
    tls_static_internal = 999;
    tls_public_default += volatile_counter;
    
    /* Take address to force TREE_USED */
    volatile int *p = &tls_weak_hidden;
    (void)p;
}

/* Helper functions that take TLS addresses */
NOINLINE void use_tls_weak_hidden(int *out) {
    volatile int *volatile_ptr = &tls_weak_hidden;
    *out = *volatile_ptr + 1;
    volatile_counter++;
}

NOINLINE void use_tls_public_default(int *out) {
    /* Complex enough to not be optimized away */
    for (volatile int i = 0; i < 3; i = i + 1) {
        *out += tls_public_default + i;
    }
    volatile_counter++;
}

NOINLINE void use_tls_protected(int *out) {
    /* Use through volatile pointer */
    volatile int *vp = &tls_protected;
    *out = *vp;
    volatile_counter++;
}

NOINLINE void use_tls_common(int *out) {
    /* Conditional access based on volatile */
    if (volatile_counter & 1) {
        *out = tls_common;
    } else {
        *out = tls_common + 1;
    }
    volatile_counter++;
}

NOINLINE void use_tls_static_internal(int *out) {
    /* Multiple volatile accesses */
    volatile int local = 0;
    for (volatile int i = 0; i < 2; i++) {
        local += tls_static_internal;
    }
    *out = local;
    volatile_counter++;
}

/* Function that takes multiple TLS addresses */
NOINLINE void use_tls_addresses(void **ptrs, int count) {
    /* Force TLS address usage in a loop */
    for (volatile int i = 0; i < count; i++) {
        if (i == 0) ptrs[i] = (void*)&tls_weak_hidden;
        if (i == 1) ptrs[i] = (void*)&tls_public_default;
        if (i == 2) ptrs[i] = (void*)&tls_protected;
        if (i == 3) ptrs[i] = (void*)&tls_common;
        if (i == 4) ptrs[i] = (void*)&tls_static_internal;
        if (i == 5) ptrs[i] = (void*)&Container::tls_in_struct;
    }
    volatile_counter++;
}

/* Compute checksum of all TLS values */
NOINLINE int compute_checksum(void) {
    int sum = 0;
    
    /* Access all TLS variables through their addresses */
    int *addresses[] = {
        &tls_weak_hidden,
        &tls_public_default,
        &tls_protected,
        &tls_common,
        &tls_static_internal,
        &Container::tls_in_struct
    };
    
    for (volatile int i = 0; i < 6; i++) {
        sum += *addresses[i];
        sum ^= (int)(uintptr_t)addresses[i];
    }
    
    return sum;
}

/* Block-scoped TLS variable test */
static void test_block_scoped_tls(void) {
    /* TLS in block scope - different DECL_CONTEXT */
    static __thread int tls_in_function = 555;
    volatile int *p = &tls_in_function;
    
    /* Modify it */
    tls_in_function += volatile_counter;
    
    /* Use it conditionally */
    if (volatile_counter > 0) {
        tls_in_function *= 2;
    }
}

/* C++ namespace test (compile as C++ for this) */
#ifdef __cplusplus
namespace TLS_Test {
    __thread int tls_in_namespace = 600;
    
    class LocalClass {
    public:
        __thread static int tls_in_class;
        
        NOINLINE void use_tls() {
            volatile int *p = &tls_in_class;
            tls_in_class += tls_in_namespace;
        }
    };
    
    __thread int LocalClass::tls_in_class = 700;
}
#endif

int main(void) {
    int results[5] = {0};
    void *tls_pointers[6] = {0};
    
    /* 1. Call helper functions that take TLS addresses */
    use_tls_weak_hidden(&results[0]);
    use_tls_public_default(&results[1]);
    use_tls_protected(&results[2]);
    use_tls_common(&results[3]);
    use_tls_static_internal(&results[4]);
    
    /* 2. Use function that takes multiple TLS addresses */
    use_tls_addresses(tls_pointers, 6);
    
    /* 3. Test block-scoped TLS */
    test_block_scoped_tls();
    
    /* 4. Conditional access based on volatile counter */
    for (volatile int i = 0; i < 3; i++) {
        switch (volatile_counter % 4) {
            case 0: tls_weak_hidden += i; break;
            case 1: tls_public_default -= i; break;
            case 2: tls_protected ^= i; break;
            case 3: tls_common |= i; break;
        }
        volatile_counter++;
    }
    
    /* 5. Access namespace TLS if in C++ mode */
    #ifdef __cplusplus
    {
        TLS_Test::LocalClass obj;
        obj.use_tls();
        volatile int *p1 = &TLS_Test::tls_in_namespace;
        volatile int *p2 = &TLS_Test::LocalClass::tls_in_class;
        (void)p1; (void)p2;
    }
    #endif
    
    /* 6. Compute and print checksum */
    int checksum = compute_checksum();
    
    /* Use the results to prevent optimization */
    volatile int final_result = 0;
    for (int i = 0; i < 5; i++) {
        final_result += results[i];
    }
    for (int i = 0; i < 6; i++) {
        final_result += (int)(uintptr_t)tls_pointers[i];
    }
    final_result += checksum;
    
    /* Return something based on TLS state */
    return (final_result > 0) ? 0 : 1;
}

/* External TLS definitions (somewhere else in real scenario) */
__thread int tls_dll_imported = 500;
__thread int tls_weak_external = 600;
