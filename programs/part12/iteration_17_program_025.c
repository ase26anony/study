/* test_emutls_cloning.c */
/* Compile with: gcc -O2 -flto -fno-fat-lto-objects -fvisibility=hidden -fno-common -fdump-tree-emutls test_emutls_cloning.c -o test_emutls */

/* Opaque functions to prevent optimization */
static void use(int val) { asm volatile("" : : "r"(val) : "memory"); }
static void use_ptr(void* ptr) { asm volatile("" : : "r"(ptr) : "memory"); }

/* Force separate compilation unit simulation */
#define MODULE __attribute__((cold, noinline, section(".text.module")))

/* TLS variables with various attributes */
__thread int tls_public_default = 42;                     /* Line 1: public, default visibility, initialized */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 0;
__thread int tls_internal __attribute__((visibility("internal")));
__thread int tls_protected __attribute__((visibility("protected"))) = 100;
extern __thread int tls_external;                         /* Line 6: external declaration */
__thread int tls_weak __attribute__((weak));
__thread int tls_common __attribute__((common));
__thread int tls_used __attribute__((used)) = 1;

/* DLL import/export simulation for Windows targets */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dll_import;
__declspec(dllexport) __thread int tls_dll_export = 99;
#endif

/* C++ style static member (using C compatibility) */
struct S {
    static __thread int member;
};
__thread int S::member = 77;                              /* Line 19: out-of-line definition */

/* TLS array */
__thread int array_tls[10] __attribute__((visibility("default")));

/* Namespace simulation */
static __thread int ns_var __attribute__((visibility("hidden")));

/* Module 1: Tests basic TLS operations with cloning context */
MODULE int test_module1(int seed) {
    /* Force DECL_PRESERVE_P and TREE_USED */
    tls_public_default += seed;
    tls_hidden = seed * 2;
    
    /* Take address to force preservation */
    use_ptr(&tls_public_default);
    use_ptr(&tls_hidden);
    
    /* Complex usage pattern */
    volatile int* volatile_ptr = &tls_internal;
    *volatile_ptr = seed;
    
    return tls_public_default + tls_hidden + tls_internal;
}

/* Module 2: Tests weak and common TLS variables */
MODULE int test_module2(int seed) {
    /* Force cloning of weak/common declarations */
    if (seed > 0) {
        tls_weak = seed % 100;
        tls_common = seed % 50;
    } else {
        tls_weak = -seed;
        tls_common = -seed * 2;
    }
    
    /* Mark as used through address computation */
    int* ptr1 = &tls_weak;
    int* ptr2 = &tls_common;
    use_ptr(ptr1);
    use_ptr(ptr2);
    
    /* Array TLS operations */
    for (int i = 0; i < 10; i++) {
        array_tls[i] = seed + i;
        use(array_tls[i]);
    }
    
    return tls_weak + tls_common + array_tls[0];
}

/* Module 3: Tests static member and namespace TLS */
MODULE int test_module3(int seed) {
    /* Static member access */
    S::member += seed * 3;
    
    /* Namespace variable */
    ns_var = seed * 4;
    
    /* External TLS reference (forces DECL_EXTERNAL handling) */
    extern __thread int tls_external_alt __attribute__((weak));
    tls_external_alt = seed * 5;
    
    /* Take addresses to prevent optimization */
    use_ptr(&S::member);
    use_ptr(&ns_var);
    use_ptr(&tls_external_alt);
    
    return S::member + ns_var + tls_external_alt;
}

/* Module 4: Tests visibility attribute propagation */
MODULE int test_module4(int seed) {
    /* Access all visibility variants */
    tls_protected = seed * 6;
    
    /* Create local pointer with different visibility context */
    static __thread int local_tls __attribute__((visibility("internal")));
    local_tls = seed * 7;
    
    /* Force TREE_PUBLIC flag manipulation */
    if (seed % 2) {
        use(tls_protected);
    } else {
        use(local_tls);
    }
    
    /* Complex expression to force cloning */
    int result = (tls_protected > local_tls) ? tls_protected : local_tls;
    use_ptr(&result);
    
    return result;
}

/* Module 5: Tests used attribute and preservation */
MODULE int test_module5(int seed) {
    /* Force DECL_PRESERVE_P through complex control flow */
    volatile int counter = seed;
    
    while (counter-- > 0) {
        tls_used += counter;
        use(tls_used);
    }
    
    /* Nested function to create declaration context */
    {
        static __thread int nested_tls __attribute__((used));
        nested_tls = seed * 8;
        use(nested_tls);
    }
    
    /* Address taken in loop */
    int* addrs[5];
    for (int i = 0; i < 5; i++) {
        addrs[i] = &tls_used;
        use_ptr(addrs[i]);
    }
    
    return tls_used;
}

/* Inline function that will be cloned - creates declaration merging context */
static inline __attribute__((always_inline)) 
int inline_tls_access(int* tls_var, int seed) {
    /* This inline function accesses TLS, causing cloning when inlined */
    static __thread int inline_tls __attribute__((visibility("hidden")));
    inline_tls = seed;
    
    *tls_var += inline_tls;
    return inline_tls;
}

/* Module 6: Tests inlining-induced cloning */
MODULE int test_module6(int seed) {
    int sum = 0;
    
    /* Call inline function multiple times with different TLS vars */
    sum += inline_tls_access(&tls_public_default, seed);
    sum += inline_tls_access(&tls_hidden, seed + 1);
    sum += inline_tls_access(&tls_internal, seed + 2);
    
    /* Create another TLS variable inside a block */
    {
        __thread int block_tls = seed * 9;
        sum += block_tls;
        use_ptr(&block_tls);
    }
    
    return sum;
}

/* Force external reference for testing DECL_EXTERNAL */
extern __thread int tls_external;
__thread int tls_external = 999;  /* Definition */

/* Main function orchestrates all tests */
int main() {
    volatile int seed = 42;  /* Prevent constant propagation */
    int checksum = 0;
    
    /* Run all test modules */
    checksum += test_module1(seed);
    checksum += test_module2(seed + 1);
    checksum += test_module3(seed + 2);
    checksum += test_module4(seed + 3);
    checksum += test_module5(seed + 4);
    checksum += test_module6(seed + 5);
    
    /* Final aggregation with all TLS variables */
    checksum += tls_public_default;
    checksum += tls_hidden;
    checksum += tls_internal;
    checksum += tls_protected;
    checksum += tls_weak;
    checksum += tls_common;
    checksum += tls_used;
    checksum += S::member;
    checksum += ns_var;
    checksum += tls_external;
    
    /* Print result to prevent dead code elimination */
    printf("TLS Cloning Test Checksum: %d\n", checksum);
    
    return checksum != 0 ? 0 : 1;
}
