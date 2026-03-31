/* test_tls_cloning.c - Comprehensive TLS attribute coverage test */

/* Prevent optimization from removing TLS accesses */
extern void use(int);
extern void use_ptr(void*);
static void use_impl(int x) { volatile static int sink; sink = x; }
static void use_ptr_impl(void* p) { volatile static void* sink; sink = p; }
#define use(x) use_impl(x)
#define use_ptr(p) use_ptr_impl(p)

/* Force separate compilation unit simulation */
#define MODULE __attribute__((cold, noinline, section(".text.module")))

/* TLS variables with diverse attributes */
__thread int tls_public_default;                     /* Default visibility, public */
__thread int tls_hidden __attribute__((visibility("hidden")));
__thread int tls_internal __attribute__((visibility("internal")));
__thread int tls_protected __attribute__((visibility("protected")));
extern __thread int tls_external;                    /* External declaration */
__thread int tls_weak __attribute__((weak));
__thread int tls_common __attribute__((common));
__thread int tls_used __attribute__((used)) = 42;    /* Initialized, used */
__thread int tls_dllimport __attribute__((dllimport));

/* DLL export for Windows-like targets */
#ifdef __CYGWIN__
__thread int tls_dllexport __attribute__((dllexport)) = 100;
#endif

/* TLS in structures */
struct S {
    static __thread int member;
};
__thread int S::member = 0;

/* TLS in namespace (C++ style) */
namespace N {
    __thread int ns_var = 5;
}

/* TLS array */
__thread int array_tls[10];

/* Module 1: Tests basic TLS operations */
MODULE int test_module1(void) {
    int sum = 0;
    
    tls_public_default = 1;
    tls_hidden = 2;
    tls_internal = 3;
    tls_protected = 4;
    
    sum += tls_public_default;
    sum += tls_hidden;
    sum += tls_internal;
    sum += tls_protected;
    
    /* Force usage flags */
    use(tls_public_default);
    use_ptr(&tls_hidden);
    
    return sum;
}

/* Module 2: Tests weak/common TLS */
MODULE int test_module2(void) {
    int sum = 0;
    
    /* Access weak TLS */
    if (&tls_weak != NULL) {
        tls_weak = 10;
        sum += tls_weak;
    }
    
    /* Access common TLS */
    tls_common = 20;
    sum += tls_common;
    
    /* Use external TLS (definition provided later) */
    extern __thread int tls_external_def;
    tls_external_def = 30;
    sum += tls_external_def;
    
    /* Take address to force preservation */
    volatile int* ptr = &tls_common;
    use_ptr(ptr);
    
    return sum;
}

/* Module 3: Tests structured TLS */
MODULE int test_module3(void) {
    int sum = 0;
    
    /* Structure member TLS */
    S::member = 100;
    sum += S::member;
    
    /* Namespace TLS */
    N::ns_var = 200;
    sum += N::ns_var;
    
    /* Array TLS */
    for (int i = 0; i < 10; i++) {
        array_tls[i] = i * 2;
        sum += array_tls[i];
    }
    
    /* Complex address computation */
    int* mid = &array_tls[5];
    use_ptr(mid);
    
    return sum;
}

/* Module 4: Tests DLL attributes */
MODULE int test_module4(void) {
    int sum = 0;
    
    /* Used attribute TLS */
    sum += tls_used;
    tls_used++;
    
#ifdef __CYGWIN__
    /* DLL import/export TLS */
    sum += tls_dllimport;
    tls_dllexport = sum;
    sum += tls_dllexport;
#endif
    
    /* Force multiple accesses in different contexts */
    for (volatile int i = 0; i < 3; i++) {
        sum += tls_used * i;
    }
    
    return sum;
}

/* Module 5: Inline function with TLS (triggers cloning) */
static inline __attribute__((always_inline)) 
int inline_func_with_tls(int x) {
    static __thread int local_tls = 0;
    local_tls += x;
    return local_tls;
}

MODULE int test_module5(void) {
    int sum = 0;
    
    /* Call inline function multiple times */
    sum += inline_func_with_tls(1);
    sum += inline_func_with_tls(2);
    sum += inline_func_with_tls(3);
    
    /* Different call site with different context */
    {
        volatile int seed = 42;
        sum += inline_func_with_tls(seed);
    }
    
    return sum;
}

/* Module 6: Static function that gets inlined */
static int static_func_with_tls(void) {
    __thread int func_tls __attribute__((visibility("hidden"))) = 0;
    func_tls++;
    return func_tls;
}

MODULE int test_module6(void) {
    int sum = 0;
    
    /* Multiple calls to static function */
    sum += static_func_with_tls();
    sum += static_func_with_tls();
    
    /* In different basic blocks */
    for (int i = 0; i < 2; i++) {
        sum += static_func_with_tls();
    }
    
    return sum;
}

/* External TLS definition */
__thread int tls_external_def = 0;

/* Main function orchestrates all tests */
int main(void) {
    volatile int seed = 0x1234;
    int total = 0;
    
    /* Initialize some TLS */
    tls_public_default = seed;
    tls_external_def = seed + 1;
    
    /* Run all test modules */
    total += test_module1();
    total += test_module2();
    total += test_module3();
    total += test_module4();
    total += test_module5();
    total += test_module6();
    
    /* Final mixed access pattern */
    total += tls_public_default;
    total += tls_hidden;
    total += tls_used;
    
    /* Print result to prevent optimization */
    use(total);
    
    return total != 0 ? 0 : 1;
}
