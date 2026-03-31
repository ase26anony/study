/* test_tls_cloning.c - Comprehensive TLS attribute coverage test */

/* Prevent optimization from removing TLS accesses */
extern void use(int);
extern void use_ptr(void*);
static void use_impl(int x) { volatile static int sink; sink = x; }
static void use_ptr_impl(void* p) { volatile static void* sink; sink = p; }
#define use(x) use_impl(x)
#define use_ptr(p) use_ptr_impl(p)

/* Force different compilation units simulation */
#define MODULE __attribute__((cold, noinline, section(".text.module")))

/* Windows DLL attributes simulation */
#ifdef _WIN32
#define DLL_IMPORT __declspec(dllimport)
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_IMPORT __attribute__((dllimport))
#define DLL_EXPORT __attribute__((dllexport))
#endif

/* ===== Module 1: Basic TLS with various visibilities ===== */
MODULE int test_module1(void) {
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
/* External declaration that will be defined in module3 */
extern __thread int tls_external_defined;

/* Weak TLS declaration */
__thread int tls_weak_var __attribute__((weak));

MODULE int test_module2(void) {
    /* Access external TLS (might trigger cloning during LTO) */
    tls_external_defined = 999;
    int val1 = tls_external_defined;
    
    /* Access weak TLS */
    if (&tls_weak_var) {
        tls_weak_var = val1 / 3;
    }
    
    /* Common linkage TLS */
    __thread int tls_common_var __attribute__((common));
    tls_common_var = val1 % 7;
    
    use(tls_weak_var);
    use(tls_common_var);
    
    return val1 + tls_weak_var + tls_common_var;
}

/* ===== Module 3: Defines external TLS and uses internal visibility ===== */
__thread int tls_external_defined = 123;

MODULE int test_module3(void) {
    /* Internal visibility TLS (not accessible outside module) */
    __thread int tls_internal __attribute__((visibility("internal"))) = 456;
    
    /* Used attribute TLS */
    __thread int tls_used_var __attribute__((used)) = 789;
    
    /* Complex operations to ensure preservation */
    for (volatile int i = 0; i < 3; i++) {
        tls_internal += i;
        tls_used_var -= i;
        tls_external_defined *= (i + 1);
    }
    
    /* Take addresses in different ways */
    int* ptr1 = &tls_internal;
    int* ptr2 = &tls_used_var;
    int* ptr3 = &tls_external_defined;
    
    use_ptr(ptr1);
    use_ptr(ptr2);
    use_ptr(ptr3);
    
    return tls_internal + tls_used_var + tls_external_defined;
}

/* ===== Module 4: TLS in structures and namespaces (C++ style in C) ===== */
struct Container {
    static __thread int member;  /* Static TLS member */
    __thread int instance_member; /* Instance TLS - not valid in C, but for testing */
};

/* Out-of-line definition required for static TLS member */
__thread int Container_member;  /* Simulating Container::member */

namespace_sim {
    __thread int ns_var = 333;
}

MODULE int test_module4(void) {
    /* Access static TLS member */
    Container_member = 111;
    Container_member++;
    
    /* Namespace TLS */
    namespace_sim::ns_var = 222;
    namespace_sim::ns_var *= 2;
    
    /* TLS array */
    __thread int tls_array[5];
    for (int i = 0; i < 5; i++) {
        tls_array[i] = i * Container_member;
    }
    
    int sum = 0;
    for (int i = 0; i < 5; i++) {
        sum += tls_array[i];
    }
    
    use_ptr(&Container_member);
    use_ptr(&namespace_sim::ns_var);
    use_ptr(tls_array);
    
    return Container_member + namespace_sim::ns_var + sum;
}

/* ===== Module 5: DLL import/export simulation ===== */
#ifdef _WIN32
/* Simulate DLL imported TLS */
extern __thread int tls_dll_imported;
DLL_IMPORT __thread int tls_dll_import_attr;

/* DLL exported TLS */
DLL_EXPORT __thread int tls_dll_exported = 888;
#endif

MODULE int test_module5(void) {
    int result = 0;
    
#ifdef _WIN32
    /* Access DLL-style TLS variables */
    tls_dll_exported = 777;
    result += tls_dll_exported;
    
    /* These might trigger special cloning for DLL attributes */
    use_ptr(&tls_dll_exported);
#endif
    
    /* TLS with preserved attribute (simulated) */
    __thread int tls_preserved __attribute__((used, visibility("default")));
    tls_preserved = 555;
    
    /* Force DECL_PRESERVE_P to be set */
    volatile int* volatile_ptr = &tls_preserved;
    result += *volatile_ptr;
    
    return result;
}

/* ===== Module 6: Inline function with TLS ===== */
static inline __attribute__((always_inline)) 
int inline_func_with_tls(int x) {
    /* TLS inside inline function - may get cloned */
    static __thread int tls_inline = 0;
    tls_inline += x;
    return tls_inline;
}

MODULE int test_module6(void) {
    int sum = 0;
    
    /* Call inline function multiple times from different contexts */
    sum += inline_func_with_tls(10);
    sum += inline_func_with_tls(20);
    sum += inline_func_with_tls(30);
    
    /* Another TLS in static function that gets inlined */
    static int helper(void) {
        __thread int tls_static_func = 99;
        tls_static_func++;
        return tls_static_func;
    }
    
    sum += helper();
    sum += helper();  /* Second call may trigger cloning */
    
    return sum;
}

/* ===== Module 7: Complex expression TLS usage ===== */
MODULE int test_module7(void) {
    /* TLS used in complex expressions */
    __thread int tls_complex = 1000;
    __thread int tls_another = 2000;
    
    /* Force TREE_USED flag */
    int result = (tls_complex > 500) ? tls_complex : tls_another;
    
    /* Address computations */
    int* addr1 = &tls_complex + 1;  /* Pointer arithmetic */
    int* addr2 = &tls_another;
    
    /* Conditional access based on volatile */
    volatile int flag = 1;
    if (flag) {
        result += *addr2;
    } else {
        result += *(addr1 - 1);
    }
    
    /* Loop with TLS access */
    for (volatile int i = 0; i < 2; i++) {
        tls_complex += i;
        tls_another -= i;
    }
    
    use_ptr(addr1);
    use_ptr(addr2);
    
    return result + tls_complex + tls_another;
}

/* ===== Main function ===== */
int main(void) {
    volatile int seed = 0;  /* Prevent optimization */
    int total = 0;
    
    /* Call all test modules */
    total += test_module1();
    total += test_module2();
    total += test_module3();
    total += test_module4();
    total += test_module5();
    total += test_module6();
    total += test_module7();
    
    /* Final use to prevent dead code elimination */
    use(total);
    
    /* Print result for verification */
    printf("TLS test checksum: %d\n", total);
    
    return 0;
}
