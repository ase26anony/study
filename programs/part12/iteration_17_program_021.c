/* tls_cloning_test.c */
/* Compile with: gcc -O2 -flto -fno-fat-lto-objects -fvisibility=hidden -fno-common -fdump-tree-emutls tls_cloning_test.c -o tls_test */

#include <stdio.h>
#include <stdint.h>

/* Opaque functions to prevent optimization */
static void use(int val) { /* Empty to prevent optimization removal */ }
static void use_ptr(void* ptr) { /* Empty to prevent optimization removal */ }

/* Force separate compilation unit simulation */
#define MODULE_SECTION __attribute__((section(".text.module"), cold, noinline))

/* TLS variables with various attributes */
__thread int tls_public_default = 42;  /* Default visibility, public, initialized */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 100;
__thread int tls_internal __attribute__((visibility("internal"))) = 200;
__thread int tls_protected __attribute__((visibility("protected"))) = 300;
extern __thread int tls_external;  /* External declaration */
__thread int tls_weak __attribute__((weak)) = 400;
__thread int tls_common __attribute__((used));  /* Common linkage, marked used */
__thread int tls_used_only __attribute__((used)) = 500;

/* Define the external TLS variable */
__thread int tls_external = 600;

/* For Windows DLL attributes simulation */
#ifdef __CYGWIN__
__thread int tls_dllimport __attribute__((dllimport));
__thread int tls_dllexport __attribute__((dllexport)) = 700;
#endif

/* TLS in structures */
struct Container {
    static __thread int member;
};
__thread int Container::member = 800;

/* TLS array */
__thread int array_tls[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

/* Namespace simulation for C */
namespace N {
    __thread int ns_var = 900;
}

/* Module 1: Tests basic TLS operations */
MODULE_SECTION
static int test_module1(volatile int seed) {
    int sum = 0;
    
    /* Read and write TLS variables */
    tls_public_default += seed;
    sum += tls_public_default;
    
    tls_hidden = seed * 2;
    sum += tls_hidden;
    
    /* Take address */
    use_ptr(&tls_internal);
    
    /* Array access */
    for (int i = 0; i < 10; i++) {
        array_tls[i] += i * seed;
        sum += array_tls[i];
    }
    
    return sum;
}

/* Module 2: Tests weak and common TLS */
MODULE_SECTION  
static int test_module2(volatile int seed) {
    int sum = seed;
    
    /* Force usage of weak TLS */
    if (tls_weak != 0) {
        tls_weak = seed % 100;
        sum += tls_weak;
    }
    
    /* Common TLS variable */
    tls_common = seed * 3;
    sum += tls_common;
    
    /* External TLS */
    tls_external = tls_external + seed;
    sum += tls_external;
    
    /* Take address to force preservation */
    use_ptr(&tls_used_only);
    
    return sum;
}

/* Module 3: Tests structure and namespace TLS */
MODULE_SECTION
static int test_module3(volatile int seed) {
    int sum = 0;
    
    /* Structure member TLS */
    Container::member = seed * 4;
    sum += Container::member;
    
    /* Namespace TLS */
    N::ns_var += seed;
    sum += N::ns_var;
    
    /* Protected visibility TLS */
    tls_protected = tls_protected * 2 + seed;
    sum += tls_protected;
    
    /* Complex address computation */
    int* tls_ptr = &tls_public_default;
    for (int i = 0; i < 5; i++) {
        tls_ptr[i % 2] += i;
    }
    sum += tls_public_default;
    
    return sum;
}

/* Module 4: Inline function with TLS - triggers cloning */
static inline __attribute__((always_inline)) 
int inline_func_with_tls(volatile int x) {
    /* Local static TLS-like variable in inline function */
    static __thread int local_tls = 1000;
    local_tls += x;
    
    /* Another TLS with internal linkage */
    static __thread int internal_tls __attribute__((visibility("internal"))) = 2000;
    internal_tls -= x % 10;
    
    return local_tls + internal_tls;
}

MODULE_SECTION
static int test_module4(volatile int seed) {
    int sum = 0;
    
    /* Call inline function multiple times */
    for (int i = 0; i < 3; i++) {
        sum += inline_func_with_tls(seed + i);
    }
    
    /* Access TLS from different contexts */
    {
        volatile int temp = seed;
        tls_hidden = temp * temp;
        sum += tls_hidden;
    }
    
    /* Force TREE_USED flag */
    use(tls_internal);
    
    return sum;
}

/* Module 5: Complex control flow with TLS */
MODULE_SECTION
static int test_module5(volatile int seed) {
    int sum = 0;
    volatile int control = seed;
    
    /* TLS in different branches */
    if (control % 2 == 0) {
        static __thread int branch_tls1 = 3000;
        branch_tls1 += control;
        sum += branch_tls1;
        use_ptr(&branch_tls1);
    } else {
        static __thread int branch_tls2 __attribute__((visibility("hidden"))) = 4000;
        branch_tls2 -= control;
        sum += branch_tls2;
    }
    
    /* Loop with TLS */
    for (volatile int i = 0; i < 5; i++) {
        tls_public_default += i;
        tls_weak -= i;
    }
    sum += tls_public_default + tls_weak;
    
    /* Switch with TLS */
    switch (control % 3) {
        case 0:
            tls_external = 1111;
            break;
        case 1:
            tls_external = 2222;
            break;
        case 2:
            tls_external = 3333;
            break;
    }
    sum += tls_external;
    
    return sum;
}

/* Main function that triggers all TLS operations */
int main(void) {
    volatile int seed = 12345; /* Volatile to prevent constant propagation */
    int total_sum = 0;
    
    printf("Starting TLS cloning test...\n");
    
    /* Execute all test modules */
    total_sum += test_module1(seed);
    total_sum += test_module2(seed + 1);
    total_sum += test_module3(seed + 2);
    total_sum += test_module4(seed + 3);
    total_sum += test_module5(seed + 4);
    
    /* Final TLS accesses to ensure all are used */
    total_sum += tls_public_default;
    total_sum += tls_hidden;
    total_sum += tls_internal;
    total_sum += tls_protected;
    total_sum += tls_external;
    total_sum += tls_weak;
    total_sum += tls_common;
    total_sum += tls_used_only;
    total_sum += Container::member;
    total_sum += N::ns_var;
    
    /* Array checksum */
    for (int i = 0; i < 10; i++) {
        total_sum += array_tls[i];
    }
    
    printf("Total checksum: %d\n", total_sum);
    printf("If you see this and no crash, TLS cloning likely occurred.\n");
    
    /* Dump TLS info for debugging */
    printf("TLS addresses (for debugging):\n");
    printf("  tls_public_default: %p\n", (void*)&tls_public_default);
    printf("  tls_hidden: %p\n", (void*)&tls_hidden);
    printf("  Container::member: %p\n", (void*)&Container::member);
    
    return total_sum != 0 ? 0 : 1;
}
