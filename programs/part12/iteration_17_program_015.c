/* test_tls_cloning.c - Comprehensive TLS declaration cloning test */
#include <stdio.h>
#include <stdint.h>

/* Opaque functions to prevent optimization */
static void use(int val) { 
    volatile int sink = val;
    (void)sink;
}

static void use_ptr(void* ptr) {
    volatile void* sink = ptr;
    (void)sink;
}

/* Force separate compilation unit simulation */
#define MODULE __attribute__((cold, noinline, section(".text.module")))

/* ========== Module 1: Basic TLS with mixed attributes ========== */
MODULE int module1_test(void) {
    /* Public TLS with default visibility */
    __thread int tls_public_default = 42;
    
    /* Hidden visibility TLS */
    __thread int tls_hidden __attribute__((visibility("hidden"))) = 100;
    
    /* Used attribute to force DECL_PRESERVE_P */
    __thread int tls_used __attribute__((used)) = 200;
    
    /* Operations to ensure usage */
    tls_public_default += 1;
    tls_hidden *= 2;
    tls_used -= 3;
    
    use_ptr(&tls_public_default);
    use_ptr(&tls_hidden);
    use_ptr(&tls_used);
    
    return tls_public_default + tls_hidden + tls_used;
}

/* ========== Module 2: External and weak TLS declarations ========== */
/* External declaration (forces DECL_EXTERNAL) */
extern __thread int tls_external;

/* Weak TLS definition */
__thread int tls_weak __attribute__((weak)) = 999;

MODULE int module2_test(void) {
    /* Define the external TLS variable */
    __thread int tls_external = 1234;
    
    /* Common linkage TLS */
    __thread int tls_common __attribute__((common));
    tls_common = 567;
    
    /* Operations with volatile to prevent optimization */
    volatile int seed = 1;
    if (seed) {
        tls_external += tls_weak;
        tls_weak -= tls_common;
    }
    
    use(tls_external);
    use(tls_weak);
    use(tls_common);
    
    return tls_external ^ tls_weak ^ tls_common;
}

/* ========== Module 3: Internal and protected visibility ========== */
MODULE int module3_test(void) {
    /* Internal visibility TLS */
    __thread int tls_internal __attribute__((visibility("internal"))) = 777;
    
    /* Protected visibility TLS */
    __thread int tls_protected __attribute__((visibility("protected"))) = 888;
    
    /* Complex usage pattern */
    for (volatile int i = 0; i < 3; i++) {
        tls_internal += i;
        tls_protected -= i;
    }
    
    /* Take addresses to force preservation */
    void* ptr1 = &tls_internal;
    void* ptr2 = &tls_protected;
    use_ptr(ptr1);
    use_ptr(ptr2);
    
    return tls_internal * tls_protected;
}

/* ========== Module 4: TLS in structures and namespaces (C++ style) ========== */
/* Simulate C++ namespace with static */
static struct {
    __thread int struct_member;
} tls_struct;

/* TLS array */
__thread int tls_array[10] __attribute__((visibility("hidden")));

MODULE int module4_test(void) {
    /* Initialize struct member */
    tls_struct.struct_member = 333;
    
    /* Initialize array elements */
    for (int i = 0; i < 10; i++) {
        tls_array[i] = i * 11;
    }
    
    /* Compute checksum */
    int sum = tls_struct.struct_member;
    for (int i = 0; i < 10; i++) {
        sum += tls_array[i];
    }
    
    /* Force address usage */
    use_ptr(&tls_struct);
    use_ptr(tls_array);
    
    return sum;
}

/* ========== Module 5: DLL import/export simulation ========== */
#ifdef __CYGWIN__
#define DLL_IMPORT __attribute__((dllimport))
#define DLL_EXPORT __attribute__((dllexport))
#elif defined(__MINGW32__)
#define DLL_IMPORT __attribute__((dllimport))
#define DLL_EXPORT __attribute__((dllexport))
#else
/* Simulate for non-Windows targets */
#define DLL_IMPORT __attribute__((visibility("default")))
#define DLL_EXPORT __attribute__((visibility("default")))
#endif

/* DLL exported TLS */
DLL_EXPORT __thread int tls_exported = 654;

/* DLL imported TLS declaration */
extern DLL_IMPORT __thread int tls_imported;

MODULE int module5_test(void) {
    /* Define the imported TLS */
    __thread int tls_imported = 321;
    
    /* Mix with other attributes */
    __thread int tls_mixed __attribute__((visibility("hidden"), used)) = 159;
    
    /* Complex operations */
    volatile int counter = 5;
    while (counter--) {
        tls_exported += tls_imported;
        tls_imported ^= tls_mixed;
        tls_mixed |= tls_exported;
    }
    
    use(tls_exported);
    use(tls_imported);
    use(tls_mixed);
    
    return tls_exported + tls_imported + tls_mixed;
}

/* ========== Module 6: Inline function with TLS ========== */
static inline __attribute__((always_inline)) 
int inline_func_with_tls(int x) {
    /* TLS inside inline function - may get cloned */
    static __thread int tls_inline = 0;
    tls_inline += x;
    
    /* Hidden TLS in inline function */
    __thread int tls_inline_hidden __attribute__((visibility("hidden")));
    tls_inline_hidden = x * 2;
    
    return tls_inline + tls_inline_hidden;
}

MODULE int module6_test(void) {
    int result = 0;
    
    /* Call inline function multiple times from different contexts */
    result += inline_func_with_tls(10);
    result += inline_func_with_tls(20);
    result += inline_func_with_tls(30);
    
    /* Force separate instantiation */
    {
        volatile int a = 40;
        result += inline_func_with_tls(a);
    }
    
    return result;
}

/* ========== Module 7: Static function with TLS that gets inlined ========== */
static int static_func_with_tls(void) {
    __thread int tls_static = 0;
    tls_static++;
    
    __thread int tls_static_hidden __attribute__((visibility("hidden"))) = 50;
    tls_static_hidden--;
    
    return tls_static + tls_static_hidden;
}

/* Noinline wrapper to force cloning context */
__attribute__((noinline)) 
int call_static_func_multiple_times(void) {
    int sum = 0;
    sum += static_func_with_tls();
    sum += static_func_with_tls();
    sum += static_func_with_tls();
    return sum;
}

MODULE int module7_test(void) {
    return call_static_func_multiple_times();
}

/* ========== Main function ========== */
int main(void) {
    volatile int seed = 0x12345678;
    int total = 0;
    
    /* Call all module tests */
    total += module1_test();
    total ^= module2_test();
    total += module3_test();
    total ^= module4_test();
    total += module5_test();
    total ^= module6_test();
    total += module7_test();
    
    /* Mix with volatile seed */
    total ^= seed;
    
    printf("TLS cloning test checksum: %d\n", total);
    
    /* Additional forced usage of all TLS variables */
    use_ptr(&tls_weak);
    use_ptr(&tls_exported);
    use_ptr(&tls_struct);
    use_ptr(tls_array);
    
    return total != 0 ? 0 : 1;
}
