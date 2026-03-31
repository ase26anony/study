/* builtin_hidden_visibility.c
 * Designed to trigger GCC's builtin_function_ext_scope path
 * that sets TREE_PUBLIC=1, DECL_EXTERNAL=1, VISIBILITY_HIDDEN, etc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global volatile variable to prevent compile-time optimization */
volatile int global_seed = 0;

/* ============================================
 * PHASE 1: Declare prototypes with attributes that should
 * trigger the target hook processing for built-ins
 * ============================================ */

/* Prototype 1: Full attribute combination */
int __attribute__((visibility("hidden"), extern, used, artificial, noinline, noreturn))
__hidden_builtin_full(int x);

/* Prototype 2: Hidden visibility with external linkage */
int __attribute__((visibility("hidden"), extern))
__hidden_builtin_extern(int x);

/* Prototype 3: Hidden visibility with used attribute */
int __attribute__((visibility("hidden"), used))
__hidden_builtin_used(int x);

/* Prototype 4: Hidden visibility with artificial */
int __attribute__((visibility("hidden"), artificial))
__hidden_builtin_artificial(int x);

/* Prototype 5: Just hidden visibility */
int __attribute__((visibility("hidden")))
__hidden_builtin_simple(int x);

/* ============================================
 * PHASE 2: Target-specific built-in declarations
 * These will go through TARGET_BUILTIN_DECL hook
 * ============================================ */

#ifdef __i386__
/* x86 specific built-ins */
int __attribute__((visibility("hidden"), extern, used, artificial))
__builtin_ia32_rdtsc(void);

int __attribute__((visibility("hidden"), extern))
__builtin_ia32_add(int a, int b);

void __attribute__((visibility("hidden"), used))
__builtin_ia32_clflush(const void *p);

#elif defined(__x86_64__)
/* x86_64 specific built-ins */
unsigned long long __attribute__((visibility("hidden"), extern, artificial))
__builtin_ia32_rdtsc(void);

int __attribute__((visibility("hidden"), extern, used))
__builtin_ia32_add(int a, int b);

#elif defined(__arm__)
/* ARM specific built-ins */
int __attribute__((visibility("hidden"), extern, used, artificial))
__builtin_arm_rbit(int x);

unsigned int __attribute__((visibility("hidden"), extern))
__builtin_arm_clz(unsigned int x);

#elif defined(__aarch64__)
/* AArch64 specific built-ins */
unsigned long long __attribute__((visibility("hidden"), extern, used, artificial))
__builtin_aarch64_rbitll(unsigned long long x);

#else
/* Generic fallback - declare as regular extern with hidden visibility */
int __attribute__((visibility("hidden"), extern, used, artificial))
__generic_hidden_builtin(int x);
#endif

/* ============================================
 * PHASE 3: Function pointer manipulation to force
 * compiler to process built-in declarations
 * ============================================ */

/* Volatile function pointer array to prevent optimization */
typedef int (*func_ptr_t)(int);
volatile func_ptr_t volatile_fptr = NULL;

/* Opaque use of function pointers */
static void use_function_pointer(volatile func_ptr_t fptr, int x) {
    /* This prevents dead code elimination */
    if (fptr != NULL) {
        /* Create a side effect the compiler can't optimize away */
        global_seed += (long)fptr;
    }
}

/* Non-optimizable comparison */
static int compare_pointers(volatile func_ptr_t f1, volatile func_ptr_t f2) {
    return (f1 == f2) ? 1 : 0;
}

/* ============================================
 * PHASE 4: Main function with runtime-dependent
 * code paths to force built-in processing
 * ============================================ */

int main(int argc, char *argv[]) {
    /* Use argv to create runtime-dependent behavior */
    int use_builtin = (argc > 1 && argv[1][0] != '\0') ? 1 : 0;
    
    /* Array of function pointers to process */
    volatile func_ptr_t func_array[5];
    
    /* Initialize with addresses (or NULL) - compiler must process declarations */
    func_array[0] = (func_ptr_t)__hidden_builtin_full;
    func_array[1] = (func_ptr_t)__hidden_builtin_extern;
    func_array[2] = (func_ptr_t)__hidden_builtin_used;
    func_array[3] = (func_ptr_t)__hidden_builtin_artificial;
    func_array[4] = (func_ptr_t)__hidden_builtin_simple;
    
    /* Target-specific built-in assignments */
    volatile func_ptr_t target_builtin = NULL;
    
#ifdef __i386__
    target_builtin = (func_ptr_t)__builtin_ia32_add;
#elif defined(__x86_64__)
    target_builtin = (func_ptr_t)__builtin_ia32_add;
#elif defined(__arm__)
    target_builtin = (func_ptr_t)__builtin_arm_rbit;
#elif defined(__aarch64__)
    target_builtin = (func_ptr_t)__builtin_aarch64_rbitll;
#else
    target_builtin = (func_ptr_t)__generic_hidden_builtin;
#endif
    
    /* Store in volatile pointer */
    volatile_fptr = target_builtin;
    
    /* Loop through array - forces compiler to consider all declarations */
    for (int i = 0; i < 5; i++) {
        use_function_pointer(func_array[i], i);
        
        /* Non-optimizable comparison with target built-in */
        if (compare_pointers(func_array[i], target_builtin)) {
            global_seed += 1;
        }
    }
    
    /* Runtime-dependent call through volatile pointer */
    if (use_builtin && volatile_fptr != NULL) {
        /* This should trigger built-in processing */
        int result = volatile_fptr(global_seed);
        return result & 0xFF; /* Ensure return value varies */
    }
    
    /* Opaque return value */
    return (global_seed & 0xFF) | (use_builtin << 8);
}

/* ============================================
 * PHASE 5: Dummy implementations to satisfy linker
 * These won't be called if built-ins are properly recognized
 * ============================================ */

int __hidden_builtin_full(int x) {
    return x ^ 0x55;
}

int __hidden_builtin_extern(int x) {
    return x + 1;
}

int __hidden_builtin_used(int x) {
    return x * 2;
}

int __hidden_builtin_artificial(int x) {
    return x / 2;
}

int __hidden_builtin_simple(int x) {
    return x - 1;
}

/* Generic fallback implementation */
int __generic_hidden_builtin(int x) {
    return ~x;
}
