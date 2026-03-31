/* tls_cloning_test.c */
/* Compile with: gcc -O2 -flto -fno-fat-lto-objects -fvisibility=hidden -fno-common tls_cloning_test.c -o tls_test */

#include <stdio.h>
#include <stdlib.h>

/* Opaque functions to prevent optimization */
static void use(int val) { asm volatile("" : : "r"(val) : "memory"); }
static void use_ptr(void* ptr) { asm volatile("" : : "r"(ptr) : "memory"); }

/* Force separate compilation unit simulation */
#define MODULE __attribute__((cold, noinline, section(".text.module")))

/* TLS variables with various attributes */
__thread int tls_public_default = 42;  /* Public, default visibility, initialized */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 100;
__thread int tls_internal __attribute__((visibility("internal"))) = 200;
__thread int tls_protected __attribute__((visibility("protected"))) = 300;
extern __thread int tls_external;  /* External declaration */
__thread int tls_weak __attribute__((weak)) = 400;
__thread int tls_common __attribute__((used, common));  /* Common, used */
__thread int tls_used __attribute__((used)) = 500;

/* Define the external TLS */
__thread int tls_external = 600;

/* For Windows-style attributes */
#ifdef __CYGWIN__
__declspec(dllimport) __thread int tls_dllimport;
__declspec(dllexport) __thread int tls_dllexport = 700;
#endif

/* TLS in structures */
struct S {
    static __thread int member;
};
__thread int S::member = 800;

/* TLS in namespace (C++ style in C) */
namespace N {
    __thread int ns_var __attribute__((visibility("default"))) = 900;
}

/* TLS array */
__thread int array_tls[10] __attribute__((visibility("hidden")));

/* Inline function that uses TLS - may get cloned */
static inline __attribute__((always_inline)) 
int inline_tls_access(int idx) {
    __thread static int inline_tls __attribute__((visibility("protected"))) = 1000;
    inline_tls += idx;
    return inline_tls;
}

/* Function that triggers TLS cloning through multiple instantiations */
MODULE int module1_func(int seed) {
    /* Access various TLS variables */
    tls_public_default += seed;
    tls_hidden -= seed;
    
    /* Take address to force preservation */
    use_ptr(&tls_internal);
    use_ptr(&tls_protected);
    
    /* Use external TLS */
    tls_external = seed * 2;
    
    /* Force usage of weak TLS */
    if (tls_weak != 0) {
        tls_weak = seed;
    }
    
    /* Access common TLS */
    tls_common = seed + 100;
    
    /* Use the used attribute TLS */
    use(tls_used);
    
    /* Access structure TLS */
    S::member += seed;
    
    /* Access namespace TLS */
    N::ns_var -= seed;
    
    /* Access array TLS */
    array_tls[seed % 10] = seed;
    
    /* Use inline function with TLS */
    int result = inline_tls_access(seed);
    
    /* Complex expression to prevent optimization */
    return tls_public_default + tls_hidden + tls_external + 
           tls_weak + tls_common + S::member + N::ns_var + 
           array_tls[seed % 10] + result;
}

/* Another module with different TLS usage pattern */
MODULE int module2_func(int seed) {
    /* Different access pattern to same TLS variables */
    tls_public_default *= (seed % 5) + 1;
    tls_hidden /= (seed % 3) + 1;
    
    /* Take addresses again */
    void* ptrs[] = {&tls_internal, &tls_protected, &tls_weak};
    for (int i = 0; i < 3; i++) {
        use_ptr(ptrs[i]);
    }
    
    /* Different computation */
    tls_external = (tls_external * seed) % 1000;
    
    /* Force usage through volatile */
    volatile int* volatile_ptr = &tls_common;
    *volatile_ptr = seed * 3;
    
    /* Access structure TLS differently */
    S::member = S::member ^ seed;
    
    /* Use inline function multiple times */
    int sum = 0;
    for (int i = 0; i < 3; i++) {
        sum += inline_tls_access(seed + i);
    }
    
    return tls_public_default + tls_hidden + tls_external + 
           tls_common + S::member + sum;
}

/* Function that creates local TLS context */
MODULE int module3_func(int seed) {
    /* Local static TLS - may get cloned when inlined */
    static __thread int local_static_tls __attribute__((visibility("hidden"))) = 2000;
    
    /* Thread-local with DLL attributes simulation */
    #ifdef __CYGWIN__
    tls_dllexport = seed * 10;
    use(tls_dllimport);
    #endif
    
    local_static_tls += seed * 2;
    
    /* Complex access pattern */
    __thread int local_auto_tls __attribute__((used)) = 3000;
    local_auto_tls = seed;
    
    /* Take address to force preservation */
    use_ptr(&local_static_tls);
    use_ptr(&local_auto_tls);
    
    /* Access through pointer */
    int* tls_ptr = &local_static_tls;
    for (int i = 0; i < 5; i++) {
        *tls_ptr += i;
    }
    
    return local_static_tls + local_auto_tls + seed;
}

/* Function that forces TLS usage in loops */
MODULE int module4_func(int seed) {
    int result = 0;
    
    /* Loop with TLS access - prevents dead code elimination */
    for (int i = 0; i < 10; i++) {
        tls_public_default += i;
        tls_hidden -= i;
        result += tls_public_default + tls_hidden;
        
        /* Conditional TLS access */
        if (i % 2 == 0) {
            tls_internal = i * seed;
            use(tls_internal);
        } else {
            tls_protected = i + seed;
            use(tls_protected);
        }
    }
    
    /* Nested function with TLS (GNU C extension) */
    auto int nested_func(int x) __attribute__((noinline));
    int nested_func(int x) {
        static __thread int nested_tls __attribute__((visibility("internal"))) = 4000;
        nested_tls += x;
        return nested_tls;
    }
    
    result += nested_func(seed);
    result += nested_func(seed * 2);
    
    return result;
}

/* Main function that triggers all scenarios */
int main(int argc, char** argv) {
    volatile int seed = (argc > 1) ? atoi(argv[1]) : 12345;
    int checksum = 0;
    
    /* Initialize array TLS */
    for (int i = 0; i < 10; i++) {
        array_tls[i] = i * 100;
    }
    
    /* Call all module functions */
    checksum += module1_func(seed);
    checksum += module2_func(seed + 1);
    checksum += module3_func(seed + 2);
    checksum += module4_func(seed + 3);
    
    /* Force additional TLS usage in main */
    use_ptr(&tls_public_default);
    use_ptr(&tls_hidden);
    use_ptr(&tls_internal);
    use_ptr(&tls_protected);
    use_ptr(&tls_external);
    use_ptr(&tls_weak);
    use_ptr(&tls_common);
    use_ptr(&tls_used);
    use_ptr(&S::member);
    use_ptr(&N::ns_var);
    use_ptr(array_tls);
    
    /* Final computation using all TLS variables */
    checksum += tls_public_default + tls_hidden + tls_internal + 
                tls_protected + tls_external + tls_weak + tls_common + 
                tls_used + S::member + N::ns_var;
    
    for (int i = 0; i < 10; i++) {
        checksum += array_tls[i];
    }
    
    printf("TLS test checksum: %d\n", checksum);
    return 0;
}
