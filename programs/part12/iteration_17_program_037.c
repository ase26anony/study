/* tls_cloning_test.c */
/* Compile with: gcc -O2 -flto -fno-fat-lto-objects -fvisibility=hidden -fno-common -fdump-tree-emutls tls_cloning_test.c -o tls_test */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Opaque functions to prevent optimization */
static void use(int val) { 
    volatile int sink = val;
    (void)sink;
}

static void use_ptr(void* ptr) {
    volatile void* sink = ptr;
    (void)sink;
}

/* Prevent function merging */
#define MODULE __attribute__((cold, noinline, section(".text.module")))

/* TLS variables with various attributes */
/* Public with default visibility */
__thread int tls_public_default = 42;

/* Hidden visibility */
__thread int tls_hidden __attribute__((visibility("hidden")));

/* Internal visibility */
__thread int tls_internal __attribute__((visibility("internal")));

/* Protected visibility */
__thread int tls_protected __attribute__((visibility("protected")));

/* External declaration */
extern __thread int tls_external;

/* Weak linkage */
__thread int tls_weak __attribute__((weak)) = 100;

/* Common linkage */
__thread int tls_common __attribute__((common));

/* Force usage marking */
__thread int tls_used __attribute__((used)) = 200;

/* DLL import simulation for Windows targets */
#ifdef __CYGWIN__
__thread int tls_dllimport __attribute__((dllimport));
__thread int tls_dllexport __attribute__((dllexport)) = 300;
#endif

/* TLS in structures */
struct S {
    static __thread int member;
};
__thread int S::member = 50;

/* TLS in namespace (C++ style simulated in C) */
static __thread int ns_var = 60;

/* TLS array */
__thread int array_tls[10];

/* Module 1: Tests basic TLS operations */
MODULE int test_module1(volatile int seed) {
    int checksum = 0;
    
    /* Read and write to various TLS variables */
    tls_public_default += seed;
    checksum += tls_public_default;
    
    tls_hidden = seed * 2;
    checksum += tls_hidden;
    
    tls_internal = seed * 3;
    checksum += tls_internal;
    
    /* Take address to force preservation */
    use_ptr(&tls_protected);
    tls_protected = seed * 4;
    checksum += tls_protected;
    
    return checksum;
}

/* Module 2: Tests external and weak TLS */
MODULE int test_module2(volatile int seed) {
    int checksum = 0;
    
    /* Define the external TLS variable */
    __thread int tls_external = 500;
    
    tls_external += seed;
    checksum += tls_external;
    
    /* Weak TLS variable */
    if (tls_weak != 0) {
        tls_weak -= seed;
        checksum += tls_weak;
    }
    
    /* Common TLS variable */
    tls_common = seed * 10;
    checksum += tls_common;
    
    /* Force usage of used attribute variable */
    tls_used += 1;
    checksum += tls_used;
    
    return checksum;
}

/* Module 3: Tests TLS in structures and arrays */
MODULE int test_module3(volatile int seed) {
    int checksum = 0;
    
    /* Structure member TLS */
    S::member += seed * 2;
    checksum += S::member;
    
    /* Namespace-like TLS */
    ns_var = seed * 3;
    checksum += ns_var;
    
    /* Array TLS operations */
    for (int i = 0; i < 10; i++) {
        array_tls[i] = seed + i;
        checksum += array_tls[i];
    }
    
    /* Take address of array */
    use_ptr(array_tls);
    
    return checksum;
}

/* Module 4: Inline function with TLS - triggers cloning */
static inline __attribute__((always_inline)) 
int inline_func_with_tls(volatile int seed) {
    /* Local TLS in inline function - may get cloned */
    static __thread int local_tls = 1000;
    local_tls += seed;
    
    /* Another TLS with visibility */
    static __thread int local_hidden_tls __attribute__((visibility("hidden"))) = 2000;
    local_hidden_tls -= seed;
    
    return local_tls + local_hidden_tls;
}

MODULE int test_module4(volatile int seed) {
    int checksum = 0;
    
    /* Call inline function multiple times */
    checksum += inline_func_with_tls(seed);
    checksum += inline_func_with_tls(seed + 1);
    checksum += inline_func_with_tls(seed + 2);
    
    /* DLL attribute tests if on Windows target */
    #ifdef __CYGWIN__
    if (seed > 0) {
        tls_dllexport = seed * 20;
        checksum += tls_dllexport;
    }
    #endif
    
    return checksum;
}

/* Module 5: Complex visibility mixing */
MODULE int test_module5(volatile int seed) {
    int checksum = 0;
    
    /* Mix visibility in same module */
    __thread int mixed_visibility __attribute__((visibility("protected"))) = 3000;
    mixed_visibility += seed;
    checksum += mixed_visibility;
    
    /* Another with internal */
    static __thread int static_internal __attribute__((visibility("internal"))) = 4000;
    static_internal -= seed;
    checksum += static_internal;
    
    /* Force TREE_USED flag */
    volatile __thread int* volatile_ptr = &tls_used;
    checksum += *volatile_ptr;
    
    return checksum;
}

/* Function that returns TLS variable address - may trigger cloning */
static __thread int tls_for_address = 9999;
MODULE void* get_tls_address(void) {
    return &tls_for_address;
}

/* Main test driver */
int main(void) {
    volatile int seed = 42; /* Volatile to prevent constant propagation */
    int total_checksum = 0;
    
    /* Initialize array TLS */
    for (int i = 0; i < 10; i++) {
        array_tls[i] = i * 10;
    }
    
    /* Run all test modules */
    total_checksum += test_module1(seed);
    total_checksum += test_module2(seed + 1);
    total_checksum += test_module3(seed + 2);
    total_checksum += test_module4(seed + 3);
    total_checksum += test_module5(seed + 4);
    
    /* Force usage of address function */
    void* addr = get_tls_address();
    use_ptr(addr);
    
    /* Additional complex scenario: TLS in loop with volatile */
    for (volatile int i = 0; i < 5; i++) {
        tls_public_default += i;
        tls_hidden -= i;
        use(tls_public_default);
        use(tls_hidden);
    }
    
    /* Print result to prevent dead code elimination */
    printf("TLS test checksum: %d\n", total_checksum);
    
    return total_checksum != 0 ? 0 : 1;
}
