/* tls_cloning_test.c */
/* Compile with: gcc -O2 -flto -fno-fat-lto-objects -fvisibility=hidden -fno-common tls_cloning_test.c -o tls_test */

#include <stdio.h>
#include <stdlib.h>

/* Opaque functions to prevent optimization */
static void use(int val) { /* Empty to prevent optimization */ }
static void use_ptr(void* ptr) { /* Empty to prevent optimization */ }
static void* escape_ptr;

/* Module 1: Basic TLS variables with various attributes */
__attribute__((cold, noinline, section(".text.module1")))
int test_module1(void) {
    /* Public TLS with default visibility */
    __thread int tls_public_default = 42;
    
    /* Hidden visibility TLS */
    __thread int tls_hidden __attribute__((visibility("hidden"))) = 100;
    
    /* Used attribute to force DECL_PRESERVE_P */
    __thread int tls_used __attribute__((used)) = 200;
    
    int sum = 0;
    tls_public_default += 1;
    tls_hidden *= 2;
    tls_used -= 50;
    
    sum += tls_public_default;
    sum += tls_hidden;
    sum += tls_used;
    
    /* Take addresses to force usage */
    use_ptr(&tls_public_default);
    use_ptr(&tls_hidden);
    escape_ptr = &tls_used;
    
    return sum;
}

/* Module 2: External and weak TLS declarations */
__attribute__((cold, noinline, section(".text.module2")))
int test_module2(void) {
    /* External declaration (DECL_EXTERNAL) */
    extern __thread int tls_external;
    
    /* Weak TLS variable */
    __thread int tls_weak __attribute__((weak)) = 300;
    
    /* Common linkage TLS */
    __thread int tls_common __attribute__((common));
    
    int sum = 0;
    tls_common = 500;  /* Initialize common variable */
    
    if (&tls_external) {  /* Force reference to external */
        sum += 1;
    }
    
    tls_weak += 10;
    tls_common /= 2;
    
    sum += tls_weak;
    sum += tls_common;
    
    /* Force cloning by taking address in different context */
    static void (*volatile fn)(void*) = use_ptr;
    fn(&tls_weak);
    fn(&tls_common);
    
    return sum;
}

/* Define the external TLS variable */
__thread int tls_external = 999;

/* Module 3: Protected and internal visibility */
__attribute__((cold, noinline, section(".text.module3")))
int test_module3(void) {
    /* Protected visibility */
    __thread int tls_protected __attribute__((visibility("protected"))) = 777;
    
    /* Internal visibility */
    __thread int tls_internal __attribute__((visibility("internal"))) = 888;
    
    int sum = 0;
    
    /* Complex usage pattern to force cloning */
    for (volatile int i = 0; i < 3; i++) {
        tls_protected += i;
        tls_internal -= i;
    }
    
    sum += tls_protected;
    sum += tls_internal;
    
    /* Address computation */
    int* ptr1 = &tls_protected + 1;
    int* ptr2 = &tls_internal - 1;
    use_ptr(ptr1);
    use_ptr(ptr2);
    
    return sum;
}

/* Module 4: TLS in structures and namespaces (C++ style in C) */
struct Container {
    static __thread int member;  /* Static TLS member */
};

/* Out-of-line definition required */
__thread int Container_member __attribute__((alias("Container::member")));
__thread int Container::member = 1234;

/* Namespace simulation */
typedef struct { int value; } Namespace_N;
static __thread Namespace_N N_var = {4321};

__attribute__((cold, noinline, section(".text.module4")))
int test_module4(void) {
    /* TLS array */
    __thread int array_tls[10];
    
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < 10; i++) {
        array_tls[i] = i * 100;
    }
    
    /* Use structure TLS member */
    Container::member += 100;
    N_var.value -= 50;
    
    /* Compute with array */
    for (int i = 0; i < 10; i++) {
        sum += array_tls[i];
    }
    
    sum += Container::member;
    sum += N_var.value;
    
    /* Force address taking */
    use_ptr(array_tls);
    use_ptr(&Container::member);
    
    return sum;
}

/* Module 5: DLL import/export simulation for Windows-like targets */
#ifdef __CYGWIN__  /* Simulate Windows attributes */
    #define DLL_IMPORT __attribute__((dllimport))
    #define DLL_EXPORT __attribute__((dllexport))
#else
    /* For non-Windows, use visibility attributes that might trigger similar paths */
    #define DLL_IMPORT __attribute__((visibility("default")))
    #define DLL_EXPORT __attribute__((visibility("default")))
#endif

/* Simulated imported TLS */
extern DLL_IMPORT __thread int imported_tls;

/* Exported TLS */
DLL_EXPORT __thread int exported_tls = 5555;

__attribute__((cold, noinline, section(".text.module5")))
int test_module5(void) {
    int sum = 0;
    
    /* Use imported TLS (if available) */
    if (&imported_tls) {
        sum += 1000;
    }
    
    /* Use exported TLS */
    exported_tls += 111;
    sum += exported_tls;
    
    /* Force DECL_DLLIMPORT_P handling */
    use_ptr(&exported_tls);
    
    return sum;
}

/* Define imported TLS for non-Windows or when linking */
__thread int imported_tls = 2222;

/* Module 6: Inline function with TLS to force cloning */
static inline __attribute__((always_inline))
int inline_helper(int x) {
    /* TLS inside inline function - may get cloned */
    static __thread int inline_tls = 9999;
    inline_tls += x;
    return inline_tls;
}

__attribute__((cold, noinline, section(".text.module6")))
int test_module6(void) {
    int sum = 0;
    
    /* Call inline function multiple times with different contexts */
    sum += inline_helper(1);
    sum += inline_helper(2);
    sum += inline_helper(3);
    
    /* Force different optimization contexts */
    {
        volatile int v = 10;
        sum += inline_helper(v);
    }
    
    return sum;
}

/* Module 7: TLS with volatile and complex access patterns */
__attribute__((cold, noinline, section(".text.module7")))
int test_module7(void) {
    /* Volatile TLS to prevent optimization */
    __thread volatile int tls_volatile = 50;
    
    /* TLS with multiple attributes */
    __thread int tls_multi __attribute__((used, visibility("hidden"))) = 100;
    
    int sum = 0;
    
    /* Complex access pattern */
    for (volatile int i = 0; i < 5; i++) {
        tls_volatile = tls_volatile + i + tls_multi;
        tls_multi = tls_multi - i;
        
        /* Conditional based on volatile to prevent dead code elimination */
        if (tls_volatile > 100) {
            sum += tls_volatile;
        } else {
            sum += tls_multi;
        }
    }
    
    /* Pointer chain */
    int* ptr = (int*)&tls_volatile;
    int** pptr = &ptr;
    use_ptr(pptr);
    
    return sum;
}

/* Main function that triggers all test modules */
int main(void) {
    volatile unsigned int seed = 0x12345678;
    int total = 0;
    
    /* Call all test modules to trigger TLS declaration cloning */
    total += test_module1();
    total += test_module2();
    total += test_module3();
    total += test_module4();
    total += test_module5();
    total += test_module6();
    total += test_module7();
    
    /* Use volatile to prevent optimization */
    if (seed > 0) {
        printf("TLS test checksum: %d\n", total);
    } else {
        printf("Unexpected path\n");
    }
    
    return 0;
}
