/* test_tls_cloning.c - Comprehensive TLS declaration cloning test */

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
__thread int tls_public_default = 42;                     /* public, default visibility, initialized */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 100;
__thread int tls_internal __attribute__((visibility("internal")));
__thread int tls_protected __attribute__((visibility("protected")));
extern __thread int tls_external;                         /* external declaration */
__thread int tls_weak __attribute__((weak));              /* weak linkage */
__thread int tls_common __attribute__((common));          /* common linkage */
__thread int tls_used __attribute__((used));              /* force usage marking */

/* DLL import/export simulation for Windows targets */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
__declspec(dllexport) __thread int tls_dllexport = 99;
#elif defined(__CYGWIN__) || defined(__MINGW32__)
__attribute__((dllimport)) __thread int tls_dllimport;
__attribute__((dllexport)) __thread int tls_dllexport = 99;
#endif

/* TLS in structures and namespaces */
struct S {
    static __thread int member;  /* C++ style, will need definition */
};
__thread int S::member = 77;     /* out-of-line definition */

namespace N {
    __thread int ns_var = 123;
}

/* TLS array */
__thread int array_tls[10];

/* Module 1: Tests basic TLS operations with visibility attributes */
MODULE int test_module1(int seed) {
    int sum = seed;
    
    /* Access various TLS variables */
    tls_public_default += seed;
    sum += tls_public_default;
    
    tls_hidden -= seed;
    sum += tls_hidden;
    
    tls_internal = seed * 2;
    sum += tls_internal;
    
    /* Take address to force preservation */
    use_ptr(&tls_protected);
    tls_protected = sum;
    
    /* Access external TLS (definition provided later) */
    extern __thread int tls_external_def;
    tls_external_def = sum;
    sum += tls_external_def;
    
    return sum;
}

/* Module 2: Tests weak and common TLS variables */
MODULE int test_module2(int seed) {
    int sum = seed;
    
    /* Weak TLS variable */
    if (tls_weak == 0) {
        tls_weak = seed + 1;
    }
    sum += tls_weak;
    
    /* Common TLS variable */
    tls_common = seed * 3;
    sum += tls_common;
    
    /* Used attribute TLS */
    tls_used = sum;
    use(tls_used);
    
    /* Take address of multiple TLS variables */
    void* addrs[] = {&tls_weak, &tls_common, &tls_used};
    for (int i = 0; i < 3; i++) {
        use_ptr(addrs[i]);
    }
    
    return sum;
}

/* Module 3: Tests TLS in structures and namespaces */
MODULE int test_module3(int seed) {
    int sum = seed;
    
    /* Structure static member */
    S::member += seed;
    sum += S::member;
    
    /* Namespace TLS */
    N::ns_var -= seed;
    sum += N::ns_var;
    
    /* TLS array operations */
    for (int i = 0; i < 10; i++) {
        array_tls[i] = seed + i;
        sum += array_tls[i];
    }
    
    /* Complex address computation */
    use_ptr(&array_tls[5]);
    use_ptr(&S::member);
    use_ptr(&N::ns_var);
    
    return sum;
}

/* Module 4: Inline function with TLS to trigger cloning */
static inline __attribute__((always_inline)) 
int inline_func_with_tls(int x, int* counter) {
    /* Local TLS-like variable simulation */
    static __thread int local_tls = 0;
    local_tls += x;
    *counter += local_tls;
    return local_tls;
}

MODULE int test_module4(int seed) {
    int sum = seed;
    int counter = 0;
    
    /* Call inline function multiple times to potentially clone TLS context */
    for (int i = 0; i < 5; i++) {
        sum += inline_func_with_tls(seed + i, &counter);
    }
    
    sum += counter;
    
    /* Access DLL-related TLS if defined */
#ifdef tls_dllexport
    tls_dllexport = sum;
    sum += tls_dllexport;
#endif
    
#ifdef tls_dllimport
    use(tls_dllimport);  /* Just use, don't define here */
#endif
    
    return sum;
}

/* Module 5: Complex control flow with TLS */
MODULE int test_module5(int seed) {
    int sum = seed;
    volatile int condition = seed;  /* Prevent optimization */
    
    /* TLS access in conditional branches */
    if (condition & 1) {
        tls_public_default *= 2;
        sum += tls_public_default;
    } else {
        tls_hidden /= 2;
        sum += tls_hidden;
    }
    
    /* Loop with TLS modification */
    for (int i = 0; i < (condition & 0xF); i++) {
        tls_internal++;
        sum += tls_internal;
    }
    
    /* Switch with TLS */
    switch (condition % 4) {
        case 0: tls_protected = sum; break;
        case 1: tls_weak = sum; break;
        case 2: tls_common = sum; break;
        case 3: tls_used = sum; break;
    }
    
    /* Take address in complex expression */
    void* ptr = (condition & 2) ? &tls_public_default : &tls_hidden;
    use_ptr(ptr);
    
    return sum;
}

/* External TLS definition */
__thread int tls_external_def = 999;

/* Windows DLL TLS definitions */
#ifdef _WIN32
__thread int tls_dllimport = 111;
#elif defined(__CYGWIN__) || defined(__MINGW32__)
__thread int tls_dllimport = 111;
#endif

/* Main function that triggers all test modules */
int main() {
    volatile int seed = 12345;  /* Prevent constant propagation */
    int result = 0;
    
    /* Run all test modules */
    result += test_module1(seed);
    result += test_module2(seed + 1);
    result += test_module3(seed + 2);
    result += test_module4(seed + 3);
    result += test_module5(seed + 4);
    
    /* Additional direct TLS manipulation */
    tls_public_default = result;
    tls_hidden = result ^ 0xAAAA;
    tls_internal = result * 3;
    
    /* Force usage of all TLS variable addresses */
    void* all_tls_ptrs[] = {
        &tls_public_default, &tls_hidden, &tls_internal,
        &tls_protected, &tls_external_def, &tls_weak,
        &tls_common, &tls_used, &S::member, &N::ns_var,
        array_tls
    };
    
    for (unsigned i = 0; i < sizeof(all_tls_ptrs)/sizeof(all_tls_ptrs[0]); i++) {
        use_ptr(all_tls_ptrs[i]);
    }
    
    /* Final computation using TLS values */
    result = tls_public_default + tls_hidden + tls_internal + 
             tls_protected + tls_external_def + tls_weak +
             tls_common + tls_used + S::member + N::ns_var;
    
    for (int i = 0; i < 10; i++) {
        result += array_tls[i];
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return result != 0 ? 0 : 1;
}
