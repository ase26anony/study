/* tls_cloning_test.c - Test TLS declaration cloning for coverage */
#include <stdio.h>
#include <stdint.h>

/* Opaque functions to prevent optimization */
static void use(int val) { 
    volatile static int sink; 
    sink = val; 
}

static void use_ptr(void* ptr) { 
    volatile static void* sink; 
    sink = ptr; 
}

/* Prevent inlining/optimization attributes */
#define NOOPT __attribute__((noinline, noipa, cold))
#define USED __attribute__((used))

/* Module 1: Basic TLS variables with various attributes */
NOOPT int module1_test(void) {
    /* Public TLS with default visibility */
    __thread int tls_public_default = 42;
    
    /* Hidden visibility TLS */
    __thread int tls_hidden __attribute__((visibility("hidden"))) = 100;
    
    /* Used attribute to force preservation */
    __thread int tls_used __attribute__((used)) = 200;
    
    /* Operations to ensure variables are accessed */
    tls_public_default += 1;
    tls_hidden *= 2;
    tls_used -= 3;
    
    use_ptr(&tls_public_default);
    use_ptr(&tls_hidden);
    use_ptr(&tls_used);
    
    return tls_public_default + tls_hidden + tls_used;
}

/* Module 2: External and weak TLS declarations */
NOOPT int module2_test(void) {
    /* External declaration (should trigger DECL_EXTERNAL) */
    extern __thread int tls_external;
    
    /* Weak TLS variable */
    __thread int tls_weak __attribute__((weak)) = 300;
    
    /* Common linkage TLS */
    __thread int tls_common __attribute__((common));
    
    /* Operations */
    tls_common = 500;
    tls_weak += tls_common;
    
    /* Take address to force usage */
    void* ptrs[] = { &tls_external, &tls_weak, &tls_common };
    for (int i = 0; i < 3; i++) {
        use_ptr(ptrs[i]);
    }
    
    return tls_weak + tls_common;
}

/* Module 3: Different visibility modes */
NOOPT int module3_test(void) {
    /* Internal visibility */
    __thread int tls_internal __attribute__((visibility("internal"))) = 600;
    
    /* Protected visibility */
    __thread int tls_protected __attribute__((visibility("protected"))) = 700;
    
    /* Default visibility but marked preserve */
    __thread int tls_preserve __attribute__((used)) = 800;
    
    /* Complex operations to ensure cloning */
    volatile int seed = 123;
    for (int i = 0; i < 10; i++) {
        if (seed & 1) {
            tls_internal += i;
        } else {
            tls_protected -= i;
        }
        seed >>= 1;
    }
    
    use(tls_internal);
    use(tls_protected);
    use(tls_preserve);
    
    return tls_internal + tls_protected + tls_preserve;
}

/* Module 4: TLS in structures and namespaces (C++ style in C) */
struct Container {
    static __thread int member;  /* Static TLS member */
};

/* Out-of-line definition required */
__thread int Container::member = 900;

/* Namespace simulation */
typedef struct { int value; } Namespace;
static __thread Namespace ns_var = { 1000 };

NOOPT int module4_test(void) {
    /* Array TLS */
    __thread int array_tls[10];
    
    /* Initialize array */
    for (int i = 0; i < 10; i++) {
        array_tls[i] = i * 100;
    }
    
    /* Access structure and namespace TLS */
    Container::member += 50;
    ns_var.value += 100;
    
    /* Compute checksum */
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += array_tls[i];
    }
    sum += Container::member;
    sum += ns_var.value;
    
    use_ptr(array_tls);
    use_ptr(&Container::member);
    use_ptr(&ns_var);
    
    return sum;
}

/* Module 5: DLL import/export simulation for Windows targets */
#ifdef __CYGWIN__ || __MINGW32__ || _WIN32
#define DLL_IMPORT __attribute__((dllimport))
#define DLL_EXPORT __attribute__((dllexport))
#else
/* Simulate for non-Windows with visibility */
#define DLL_IMPORT __attribute__((visibility("default")))
#define DLL_EXPORT __attribute__((visibility("default")))
#endif

/* Simulated imported TLS */
extern __thread int imported_tls DLL_IMPORT;

/* Exported TLS */
__thread int exported_tls DLL_EXPORT = 1500;

NOOPT int module5_test(void) {
    exported_tls += 100;
    
    /* Simulate access to imported TLS */
    int local_copy = 1600;  /* Stand-in for imported_tls */
    
    use(exported_tls);
    use(local_copy);
    
    return exported_tls + local_copy;
}

/* Module 6: Inline function with TLS to trigger cloning */
static inline NOOPT int inline_func_with_tls(int x) {
    /* TLS in inline function - may get cloned */
    __thread int tls_inline __attribute__((visibility("hidden"))) = 1700;
    
    tls_inline += x;
    use(tls_inline);
    
    return tls_inline;
}

NOOPT int module6_test(void) {
    int sum = 0;
    
    /* Call inline function multiple times from different contexts */
    sum += inline_func_with_tls(1);
    sum += inline_func_with_tls(2);
    sum += inline_func_with_tls(3);
    
    /* Also define TLS in static function */
    static int helper(void) {
        __thread int tls_static_func = 1800;
        tls_static_func += 10;
        return tls_static_func;
    }
    
    sum += helper();
    sum += helper();  /* Second call may trigger cloning */
    
    return sum;
}

/* Module 7: Complex TLS usage patterns */
NOOPT int module7_test(void) {
    /* TLS with thread-specific computation */
    __thread int tls_computed = 1900;
    __thread int tls_temp __attribute__((visibility("internal")));
    
    /* Volatile to prevent optimization */
    volatile int iterations = 5;
    
    for (volatile int i = 0; i < iterations; i++) {
        tls_temp = tls_computed * (i + 1);
        tls_computed = tls_temp / 2;
        
        /* Conditional based on TLS value */
        if (tls_computed % 2 == 0) {
            __thread int tls_even __attribute__((used)) = 2000;
            tls_even += tls_computed;
            use(tls_even);
        } else {
            __thread int tls_odd __attribute__((visibility("hidden"))) = 2100;
            tls_odd -= tls_computed;
            use(tls_odd);
        }
    }
    
    /* Address taken in loop */
    int* ptrs[2];
    ptrs[0] = &tls_computed;
    ptrs[1] = &tls_temp;
    
    for (int i = 0; i < 2; i++) {
        use_ptr(ptrs[i]);
    }
    
    return tls_computed + tls_temp;
}

/* Main function that runs all tests */
int main(void) {
    volatile int seed = 0x12345678;  /* Prevent optimization */
    int total = 0;
    
    /* Run all module tests */
    total += module1_test();
    total += module2_test();
    total += module3_test();
    total += module4_test();
    total += module5_test();
    total += module6_test();
    total += module7_test();
    
    /* Mix in seed to prevent constant folding */
    total ^= seed;
    
    printf("TLS cloning test result: %d\n", total);
    
    /* External TLS definition (for module2) */
    __thread int tls_external = 400;
    
    return 0;
}
