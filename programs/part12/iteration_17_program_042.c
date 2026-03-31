/* test_tls_cloning.c - Comprehensive TLS declaration cloning test */
#include <stdio.h>
#include <stdint.h>

/* Opaque functions to prevent optimization */
static void use(int val) { (void)val; }
static void use_ptr(void* ptr) { (void)ptr; }

/* Volatile seed to prevent constant folding */
static volatile int seed = 42;

/* ========== MODULE 1: Basic TLS with various visibilities ========== */
__attribute__((cold, noinline, section(".text.module1")))
static int test_module1(void) {
    /* Public TLS with default visibility */
    __thread int tls_public_default = seed;
    
    /* Hidden visibility TLS */
    __thread int tls_hidden __attribute__((visibility("hidden"))) = seed + 1;
    
    /* Internal visibility TLS */
    __thread int tls_internal __attribute__((visibility("internal"))) = seed + 2;
    
    /* Protected visibility TLS */
    __thread int tls_protected __attribute__((visibility("protected"))) = seed + 3;
    
    /* Force usage and take addresses */
    tls_public_default++;
    tls_hidden *= 2;
    tls_internal -= seed;
    tls_protected ^= 0x55;
    
    use_ptr(&tls_public_default);
    use_ptr(&tls_hidden);
    use_ptr(&tls_internal);
    use_ptr(&tls_protected);
    
    return tls_public_default + tls_hidden + tls_internal + tls_protected;
}

/* ========== MODULE 2: External and weak TLS declarations ========== */
/* External declaration (will be defined in module3) */
extern __thread int tls_external;

/* Weak TLS declaration */
__thread int tls_weak __attribute__((weak)) = 100;

/* Common TLS */
__thread int tls_common __attribute__((common));

__attribute__((cold, noinline, section(".text.module2")))
static int test_module2(void) {
    int sum = 0;
    
    /* Access weak TLS */
    tls_weak = seed * 2;
    sum += tls_weak;
    
    /* Access common TLS */
    tls_common = seed + 1000;
    sum += tls_common;
    
    /* External TLS (will be resolved later) */
    sum += tls_external;
    
    /* Force preservation by taking address */
    use_ptr(&tls_weak);
    use_ptr(&tls_common);
    
    return sum;
}

/* ========== MODULE 3: External TLS definition ========== */
/* Define the external TLS from module2 */
__thread int tls_external = 999;

/* Used attribute TLS */
__thread int tls_used __attribute__((used)) = 1234;

__attribute__((cold, noinline, section(".text.module3")))
static int test_module3(void) {
    tls_external = seed * 3;
    tls_used += seed;
    
    /* Complex address computation */
    int* ptr = &tls_used;
    *ptr += 1;
    
    use_ptr(ptr);
    use(tls_external);
    
    return tls_external + tls_used;
}

/* ========== MODULE 4: TLS in inline functions ========== */
/* Static inline function that uses TLS - may be cloned */
static inline __attribute__((always_inline)) 
int inline_tls_helper(int x) {
    static __thread int counter = 0;
    counter += x;
    return counter;
}

/* Another inline function with TLS */
inline __attribute__((gnu_inline)) 
int another_inline_tls(int y) {
    __thread int accumulator __attribute__((visibility("hidden"))) = 0;
    accumulator ^= y;
    return accumulator;
}

__attribute__((cold, noinline, section(".text.module4")))
static int test_module4(void) {
    int sum = 0;
    
    /* Call inline functions multiple times */
    for (int i = 0; i < 5; i++) {
        sum += inline_tls_helper(seed + i);
        sum += another_inline_tls(seed - i);
    }
    
    return sum;
}

/* ========== MODULE 5: TLS in structures and namespaces (C++ style) ========== */
/* Simulate C++ namespace with static */
struct NamespaceSim {
    static __thread int ns_var;
    static __thread int ns_array[4];
};

/* Define the static TLS members */
__thread int NamespaceSim::ns_var = 555;
__thread int NamespaceSim::ns_array[4] = {1, 2, 3, 4};

/* TLS in a struct */
struct Container {
    __thread int member;
    __thread char buffer[8];
};

__attribute__((cold, noinline, section(".text.module5")))
static int test_module5(void) {
    /* Access namespace-style TLS */
    NamespaceSim::ns_var = seed * 4;
    int* arr_ptr = &NamespaceSim::ns_array[0];
    arr_ptr[0] = seed;
    
    /* Instance TLS */
    static __thread Container container;
    container.member = seed + 50;
    container.buffer[0] = seed & 0xFF;
    
    use_ptr(&NamespaceSim::ns_var);
    use_ptr(arr_ptr);
    use_ptr(&container);
    
    return NamespaceSim::ns_var + arr_ptr[0] + container.member;
}

/* ========== MODULE 6: DLL import/export simulation ========== */
#ifdef __CYGWIN__ || __MINGW32__
/* Simulate Windows DLL attributes */
__thread int tls_dll_export __attribute__((dllexport)) = 777;
extern __thread int tls_dll_import __attribute__((dllimport));
#else
/* For non-Windows, use default visibility */
__thread int tls_dll_export __attribute__((visibility("default"))) = 777;
extern __thread int tls_dll_import;
#endif

__thread int tls_dll_import = 888; /* Actually define it */

__attribute__((cold, noinline, section(".text.module6")))
static int test_module6(void) {
    tls_dll_export += seed;
    tls_dll_import -= seed;
    
    /* Force complex usage pattern */
    if (seed & 1) {
        tls_dll_export ^= tls_dll_import;
    } else {
        tls_dll_import |= tls_dll_export;
    }
    
    use(tls_dll_export);
    use(tls_dll_import);
    
    return tls_dll_export + tls_dll_import;
}

/* ========== MODULE 7: TLS with preservation triggers ========== */
/* Function that takes address of TLS and doesn't inline */
__attribute__((noipa, noinline))
static void* get_tls_address(void) {
    static __thread int preserved_tls __attribute__((used)) = 42;
    preserved_tls += seed;
    return &preserved_tls;
}

/* TLS with volatile-like access pattern */
__thread int volatile_tls_access;

__attribute__((cold, noinline, section(".text.module7")))
static int test_module7(void) {
    /* Force DECL_PRESERVE_P by taking address in non-inlinable function */
    void* addr = get_tls_address();
    use_ptr(addr);
    
    /* Complex access pattern that's hard to optimize away */
    for (volatile int i = 0; i < 3; i++) {
        volatile_tls_access = seed << i;
        use(volatile_tls_access);
    }
    
    return volatile_tls_access + (int)(intptr_t)addr;
}

/* ========== MAIN FUNCTION ========== */
int main(void) {
    int total_checksum = 0;
    
    /* Run all test modules */
    total_checksum += test_module1();
    total_checksum += test_module2();
    total_checksum += test_module3();
    total_checksum += test_module4();
    total_checksum += test_module5();
    total_checksum += test_module6();
    total_checksum += test_module7();
    
    /* Final volatile store to prevent dead code elimination */
    static volatile int result;
    result = total_checksum;
    
    printf("TLS cloning test checksum: %d\n", total_checksum);
    return 0;
}
