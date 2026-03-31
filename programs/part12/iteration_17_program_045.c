/* tls_cloning_test.c - Test TLS declaration cloning for GCC tree-emutls.cc coverage */

/* Opaque functions to prevent optimization */
static void use(int val) { (void)val; }
static void use_ptr(void* ptr) { (void)ptr; }

/* Force separate compilation unit simulation */
#define MODULE __attribute__((cold, noinline, section(".text.module")))

/* TLS variables with various attributes */
__thread int tls_public_default;                     /* Default visibility, public linkage */
__thread int tls_hidden __attribute__((visibility("hidden")));
__thread int tls_internal __attribute__((visibility("internal")));
__thread int tls_protected __attribute__((visibility("protected")));
extern __thread int tls_external;                    /* External declaration */
__thread int tls_weak __attribute__((weak));
__thread int tls_common __attribute__((common));
__thread int tls_used __attribute__((used)) = 42;    /* Force used attribute with initializer */

/* DLL import/export simulation for Windows targets */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
__declspec(dllexport) __thread int tls_dllexport = 100;
#endif

/* TLS in structures */
struct S {
    static __thread int member;  /* Static TLS member */
};
__thread int S::member;          /* Out-of-line definition */

/* TLS in namespace (C++ style, but in C for compatibility) */
typedef struct { int dummy; } namespace_N;
__thread int N_var __attribute__((visibility("default")));

/* TLS array */
__thread int array_tls[10] __attribute__((visibility("hidden")));

/* Module 1: Tests basic TLS operations with mixed attributes */
MODULE int test_module1(volatile int seed) {
    int sum = 0;
    
    /* Access various TLS variables */
    tls_public_default = seed;
    tls_hidden = seed + 1;
    tls_internal = seed + 2;
    tls_protected = seed + 3;
    
    /* Force usage through pointer */
    use_ptr(&tls_public_default);
    use_ptr(&tls_hidden);
    
    /* Read back and compute checksum */
    sum += tls_public_default;
    sum += tls_hidden;
    sum += tls_internal;
    sum += tls_protected;
    
    /* Access weak TLS */
    if (tls_weak) {
        sum += tls_weak;
    }
    
    return sum;
}

/* Module 2: Tests TLS in inline functions that may get cloned */
static inline __attribute__((always_inline)) 
int inline_tls_access(int idx) {
    /* This inline function accesses TLS and may be cloned */
    static __thread int inline_tls __attribute__((visibility("default")));
    inline_tls += idx;
    return inline_tls;
}

MODULE int test_module2(volatile int seed) {
    int sum = 0;
    
    /* Call inline function multiple times */
    for (int i = 0; i < 5; i++) {
        sum += inline_tls_access(seed + i);
    }
    
    /* Access structure TLS member */
    S::member = seed * 2;
    sum += S::member;
    
    /* Access namespace TLS */
    N_var = seed * 3;
    sum += N_var;
    
    return sum;
}

/* Module 3: Tests TLS arrays and complex expressions */
MODULE int test_module3(volatile int seed) {
    int sum = 0;
    
    /* Initialize TLS array */
    for (int i = 0; i < 10; i++) {
        array_tls[i] = seed + i;
    }
    
    /* Compute sum of array elements */
    for (int i = 0; i < 10; i++) {
        sum += array_tls[i];
    }
    
    /* Access with volatile to prevent optimization */
    volatile __thread int volatile_tls __attribute__((visibility("hidden")));
    volatile_tls = seed * 5;
    sum += volatile_tls;
    
    /* Force DECL_PRESERVE_P usage */
    use_ptr((void*)&volatile_tls);
    
    return sum;
}

/* Module 4: Tests external TLS and common linkage */
MODULE int test_module4(volatile int seed) {
    int sum = 0;
    
    /* Define the external TLS variable */
    __thread int tls_external = seed * 7;
    
    /* Access common TLS */
    tls_common = seed * 8;
    sum += tls_common;
    
    /* Access used TLS */
    sum += tls_used;
    tls_used += seed;
    
    /* Complex expression with TLS */
    sum += (tls_external > 0) ? tls_external : -tls_external;
    
    return sum;
}

/* Module 5: Tests TLS in static functions that may be inlined */
static int static_func_with_tls(int x) __attribute__((noinline));
static int static_func_with_tls(int x) {
    static __thread int static_tls __attribute__((visibility("protected")));
    static_tls += x;
    return static_tls;
}

MODULE int test_module5(volatile int seed) {
    int sum = 0;
    
    /* Call static function multiple times */
    sum += static_func_with_tls(seed);
    sum += static_func_with_tls(seed + 1);
    sum += static_func_with_tls(seed + 2);
    
    /* Create local TLS in loop */
    for (int i = 0; i < 3; i++) {
        static __thread int loop_tls __attribute__((visibility("internal")));
        loop_tls = seed + i * 10;
        sum += loop_tls;
    }
    
    return sum;
}

/* Module 6: Tests Windows-specific attributes */
MODULE int test_module6(volatile int seed) {
    int sum = seed;
    
#ifdef _WIN32
    /* Access DLL import/export TLS variables */
    tls_dllexport = seed * 11;
    sum += tls_dllexport;
    
    if (&tls_dllimport != NULL) {
        sum += 1;  /* Just reference the address */
    }
#endif
    
    /* Force TREE_USED on all TLS variables */
    use(tls_public_default);
    use(tls_hidden);
    use(tls_internal);
    use(tls_protected);
    
    return sum;
}

/* Main function that triggers all test modules */
int main(void) {
    volatile int seed = 42;  /* Volatile to prevent constant propagation */
    int total = 0;
    
    /* Call all test modules */
    total += test_module1(seed);
    total += test_module2(seed + 1);
    total += test_module3(seed + 2);
    total += test_module4(seed + 3);
    total += test_module5(seed + 4);
    total += test_module6(seed + 5);
    
    /* Final computation using TLS variables */
    total += tls_public_default;
    total += tls_hidden;
    total += tls_internal;
    total += tls_protected;
    total += S::member;
    total += N_var;
    
    /* Print result to prevent dead code elimination */
    printf("TLS test checksum: %d\n", total);
    
    return (total > 0) ? 0 : 1;
}
