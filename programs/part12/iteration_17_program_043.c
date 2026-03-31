/* tls_cloning_test.c */
/* Compile with: gcc -O2 -flto -fno-fat-lto-objects -fvisibility=hidden -fno-common -fdump-tree-emutls tls_cloning_test.c -o tls_test */

/* Opaque functions to prevent optimization */
static void use(int x) { (void)x; }
static void use_ptr(void* p) { (void)p; }

/* Global volatile to prevent constant propagation */
static volatile int seed = 42;

/* ========== Module 1: Basic TLS with various attributes ========== */
__attribute__((cold, noinline, section(".text.module1")))
static int test_module1(void) {
    /* Public TLS with default visibility */
    __thread int tls_public_default = seed;
    
    /* Hidden visibility TLS */
    __thread int tls_hidden __attribute__((visibility("hidden"))) = seed + 1;
    
    /* Used attribute to force DECL_PRESERVE_P */
    __thread int tls_used __attribute__((used)) = seed + 2;
    
    /* Operations to ensure variables are accessed */
    tls_public_default += tls_hidden;
    tls_hidden *= tls_used;
    tls_used = tls_public_default ^ tls_hidden;
    
    use_ptr(&tls_public_default);
    use_ptr(&tls_hidden);
    use_ptr(&tls_used);
    
    return tls_public_default + tls_hidden + tls_used;
}

/* ========== Module 2: Weak and Common TLS ========== */
__attribute__((cold, noinline, section(".text.module2")))
static int test_module2(void) {
    /* Weak TLS - may trigger DECL_WEAK copying */
    __thread int tls_weak __attribute__((weak)) = seed * 2;
    
    /* Common TLS - affects DECL_COMMON */
    __thread int tls_common __attribute__((common));
    tls_common = seed * 3;
    
    /* External declaration */
    extern __thread int tls_external;
    tls_external = seed * 4;
    
    /* Force usage */
    int sum = tls_weak + tls_common;
    use(tls_external);
    
    use_ptr(&tls_weak);
    use_ptr(&tls_common);
    
    return sum;
}

/* External TLS definition (for module2) */
__thread int tls_external;

/* ========== Module 3: Internal and Protected visibility ========== */
__attribute__((cold, noinline, section(".text.module3")))
static int test_module3(void) {
    /* Internal visibility */
    __thread int tls_internal __attribute__((visibility("internal"))) = seed * 5;
    
    /* Protected visibility */
    __thread int tls_protected __attribute__((visibility("protected"))) = seed * 6;
    
    /* Complex operations to prevent optimization */
    for (int i = 0; i < 10; i++) {
        tls_internal += i;
        tls_protected ^= i;
    }
    
    use_ptr(&tls_internal);
    use_ptr(&tls_protected);
    
    return tls_internal * tls_protected;
}

/* ========== Module 4: DLL import/export simulation ========== */
#ifdef __CYGWIN__
#define DLL_IMPORT __attribute__((dllimport))
#define DLL_EXPORT __attribute__((dllexport))
#else
/* Simulate for non-Windows */
#define DLL_IMPORT __attribute__((visibility("default")))
#define DLL_EXPORT __attribute__((visibility("default")))
#endif

/* DLL exported TLS */
DLL_EXPORT __thread int tls_exported = 100;

__attribute__((cold, noinline, section(".text.module4")))
static int test_module4(void) {
    /* DLL imported TLS */
    extern DLL_IMPORT __thread int tls_imported;
    
    tls_exported += seed;
    int local_copy = tls_imported;
    
    use_ptr(&tls_exported);
    use_ptr(&tls_imported);
    
    return tls_exported + local_copy;
}

/* DLL imported TLS definition */
__thread int tls_imported = 200;

/* ========== Module 5: C++ style TLS (compile as C++ for full effect) ========== */
#ifdef __cplusplus
namespace TLS_Namespace {
    __thread int ns_var = 300;
    
    struct Container {
        static __thread int static_member;
        __thread int instance_member;
    };
}

/* Out-of-line definition required */
__thread int TLS_Namespace::Container::static_member = 400;

__attribute__((cold, noinline, section(".text.module5")))
static int test_module5(void) {
    using namespace TLS_Namespace;
    
    ns_var += seed;
    Container::static_member *= seed;
    
    Container c;
    c.instance_member = seed * 7;
    
    use_ptr(&ns_var);
    use_ptr(&Container::static_member);
    use_ptr(&c.instance_member);
    
    return ns_var + Container::static_member + c.instance_member;
}
#else
/* C version */
__attribute__((cold, noinline, section(".text.module5")))
static int test_module5(void) {
    /* TLS array */
    __thread int tls_array[10];
    
    /* Initialize array */
    for (int i = 0; i < 10; i++) {
        tls_array[i] = seed * (i + 1);
    }
    
    /* Compute checksum */
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum ^= tls_array[i];
    }
    
    use_ptr(tls_array);
    
    return sum;
}
#endif

/* ========== Module 6: Inline function with TLS ========== */
/* This inline function may get cloned when inlined into different contexts */
static inline __attribute__((always_inline)) 
int inline_tls_helper(int multiplier) {
    /* TLS in inline function - likely to be cloned */
    static __thread int inline_tls_var = 0;
    inline_tls_var += seed * multiplier;
    
    /* Another TLS with attribute */
    __thread int inline_tls_attr __attribute__((visibility("hidden")));
    inline_tls_attr = inline_tls_var ^ multiplier;
    
    use_ptr(&inline_tls_var);
    use_ptr(&inline_tls_attr);
    
    return inline_tls_var + inline_tls_attr;
}

__attribute__((cold, noinline, section(".text.module6")))
static int test_module6(void) {
    int sum = 0;
    
    /* Call inline function multiple times with different contexts */
    sum += inline_tls_helper(1);
    sum += inline_tls_helper(2);
    sum += inline_tls_helper(3);
    
    /* Force different optimization contexts */
    if (seed > 0) {
        sum += inline_tls_helper(4);
    } else {
        sum += inline_tls_helper(5);
    }
    
    return sum;
}

/* ========== Module 7: Complex TLS usage patterns ========== */
__attribute__((cold, noinline, section(".text.module7")))
static int test_module7(void) {
    /* TLS with thread-local pointer to TLS */
    __thread int* tls_ptr __attribute__((visibility("internal")));
    
    /* Target TLS for the pointer */
    __thread int tls_target __attribute__((used)) = seed * 8;
    
    /* Make pointer point to TLS */
    tls_ptr = &tls_target;
    
    /* Indirect access through TLS pointer */
    *tls_ptr += 1;
    
    /* Another level of indirection */
    __thread int** tls_pptr;
    tls_pptr = &tls_ptr;
    **tls_pptr *= 2;
    
    use_ptr(&tls_ptr);
    use_ptr(&tls_target);
    use_ptr(&tls_pptr);
    
    return tls_target + (int)(intptr_t)tls_ptr;
}

/* ========== Main function ========== */
int main(void) {
    int checksum = 0;
    
    /* Update seed to prevent constant folding */
    seed = (seed * 1103515245 + 12345) & 0x7fffffff;
    
    /* Run all test modules */
    checksum += test_module1();
    checksum += test_module2();
    checksum += test_module3();
    checksum += test_module4();
    checksum += test_module5();
    checksum += test_module6();
    checksum += test_module7();
    
    /* Use checksum to prevent dead code elimination */
    use(checksum);
    
    /* Print result (prevents optimization of entire program) */
    printf("TLS test checksum: %d\n", checksum);
    
    return checksum != 0 ? 0 : 1;
}
