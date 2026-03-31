/* test_tls_cloning.c - Comprehensive TLS attribute coverage test */
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

/* ===== Module 1: Basic TLS with various visibilities ===== */
MODULE int module1_test(void) {
    /* Public TLS with default visibility */
    __thread int tls_public_default = 42;
    
    /* Hidden visibility TLS */
    __thread int tls_hidden __attribute__((visibility("hidden"))) = 100;
    
    /* Protected visibility TLS */
    __thread int tls_protected __attribute__((visibility("protected"))) = 200;
    
    /* Force usage and address taking */
    tls_public_default++;
    tls_hidden *= 2;
    tls_protected -= 50;
    
    use_ptr(&tls_public_default);
    use_ptr(&tls_hidden);
    use_ptr(&tls_protected);
    
    return tls_public_default + tls_hidden + tls_protected;
}

/* ===== Module 2: External and weak TLS declarations ===== */
/* Forward declarations to create external references */
extern __thread int tls_external;
__thread int tls_weak __attribute__((weak));
__thread int tls_common __attribute__((common));

MODULE int module2_test(void) {
    /* Define the external TLS */
    __thread int tls_external = 999;
    
    /* Initialize weak TLS */
    tls_weak = 777;
    
    /* Common TLS (may be merged) */
    tls_common = 333;
    
    /* Complex usage pattern to force preservation */
    volatile int seed = 123;
    for (int i = 0; i < 10; i++) {
        if (seed & (1 << i)) {
            tls_external += i;
            tls_weak -= i;
            tls_common ^= i;
        }
    }
    
    use(tls_external);
    use(tls_weak);
    use(tls_common);
    
    return tls_external ^ tls_weak ^ tls_common;
}

/* ===== Module 3: TLS in structures and namespaces (C++ style in C) ===== */
/* Simulate C++ namespace with static */
static struct {
    __thread int struct_member;
} namespace_sim;

/* TLS array */
__thread int tls_array[10] __attribute__((used));

MODULE int module3_test(void) {
    /* Structure member TLS */
    namespace_sim.struct_member = 1000;
    
    /* Array TLS operations */
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        tls_array[i] = i * 10;
        sum += tls_array[i];
        use_ptr(&tls_array[i]);
    }
    
    /* Force address computation */
    int* middle = &tls_array[5];
    *middle = 999;
    
    return namespace_sim.struct_member + sum + *middle;
}

/* ===== Module 4: Internal visibility and used attribute ===== */
MODULE int module4_test(void) {
    /* Internal visibility (not exposed outside module) */
    __thread int tls_internal __attribute__((visibility("internal"))) = 555;
    
    /* Forcefully used TLS */
    __thread int tls_force_used __attribute__((used)) = 888;
    
    /* Mark as used through volatile operations */
    volatile int* ptr1 = &tls_internal;
    volatile int* ptr2 = &tls_force_used;
    
    *ptr1 += 111;
    *ptr2 -= 222;
    
    /* Opaque function calls that compiler can't analyze */
    use_ptr((void*)ptr1);
    use_ptr((void*)ptr2);
    
    return *ptr1 * *ptr2;
}

/* ===== Module 5: DLL import/export simulation ===== */
#ifdef __CYGWIN__
#define DLL_IMPORT __attribute__((dllimport))
#define DLL_EXPORT __attribute__((dllexport))
#else
/* Simulate for non-Windows with visibility attributes */
#define DLL_IMPORT __attribute__((visibility("default")))
#define DLL_EXPORT __attribute__((visibility("default")))
#endif

/* Simulated imported TLS */
extern __thread int imported_tls DLL_IMPORT;

/* Exported TLS */
__thread int exported_tls DLL_EXPORT = 1234;

MODULE int module5_test(void) {
    /* Define the imported TLS locally if not actually imported */
#ifndef __CYGWIN__
    __thread int imported_tls = 4321;
#endif
    
    /* Operations that might trigger cloning with DLL attributes */
    imported_tls = exported_tls / 2;
    exported_tls = imported_tls * 3;
    
    /* Complex control flow */
    volatile int counter = 5;
    while (counter-- > 0) {
        imported_tls++;
        exported_tls--;
        use(imported_tls);
        use(exported_tls);
    }
    
    return imported_tls + exported_tls;
}

/* ===== Module 6: Inline function with TLS ===== */
/* Static inline function that may be cloned */
static inline __attribute__((always_inline)) 
int inline_tls_helper(int x) {
    static __thread int inline_tls = 0;  /* Static TLS inside inline function */
    inline_tls += x;
    return inline_tls;
}

MODULE int module6_test(void) {
    int sum = 0;
    
    /* Call inline function multiple times - may create multiple TLS instances */
    for (int i = 0; i < 100; i++) {
        sum += inline_tls_helper(i % 10);
    }
    
    /* Another TLS in a different inline context */
    {
        static __thread int another_inline_tls __attribute__((visibility("hidden"))) = 50;
        another_inline_tls += sum % 100;
        sum += another_inline_tls;
    }
    
    return sum;
}

/* ===== Module 7: TLS with preservation triggers ===== */
/* Function that takes address of TLS and doesn't inline */
__attribute__((noipa)) 
void* get_tls_address(void) {
    __thread int preserved_tls __attribute__((visibility("protected"))) = 9999;
    return &preserved_tls;
}

MODULE int module7_test(void) {
    /* TLS that might be preserved due to address escape */
    __thread int escape_tls = 1111;
    
    /* Force address to escape through opaque function */
    void* addr = &escape_tls;
    use_ptr(addr);
    
    /* Call noipa function */
    void* preserved_addr = get_tls_address();
    use_ptr(preserved_addr);
    
    /* Volatile access pattern */
    volatile int* volatile_ptr = &escape_tls;
    for (int i = 0; i < 20; i++) {
        *volatile_ptr += i;
        if (i % 3 == 0) {
            use(*volatile_ptr);
        }
    }
    
    return escape_tls + (int)(intptr_t)preserved_addr;
}

/* ===== Main function to drive all tests ===== */
int main(void) {
    volatile int seed = 0xABCD1234;  /* Prevent constant folding */
    int result = 0;
    
    /* Run all module tests */
    result ^= module1_test();
    result ^= module2_test();
    result ^= module3_test();
    result ^= module4_test();
    result ^= module5_test();
    result ^= module6_test();
    result ^= module7_test();
    
    /* Additional global TLS with mixed attributes */
    __thread int final_tls __attribute__((visibility("hidden"), common)) = result;
    use(final_tls);
    
    printf("Result: %d\n", result ^ final_tls);
    return 0;
}
