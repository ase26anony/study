/* Compile with: gcc -O0 -march=native -fdump-tree-original -fvisibility=hidden builtin_test.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global volatile variable to prevent optimization */
volatile int global_seed = 0;

/* Function to create opaque value that compiler can't optimize away */
static int get_opaque_value(void) {
    return global_seed ^ 0x12345678;
}

/* ============================================
   DECLARATION OF BUILT-IN LIKE FUNCTIONS WITH
   VISIBILITY AND LINKAGE ATTRIBUTES
   ============================================ */

/* Prototype 1: Full attribute combination targeting the uncovered block */
int __attribute__((visibility("hidden"), extern, used, artificial, noinline, noclone))
__hidden_builtin_1(int x);

/* Prototype 2: Different attribute order */
int __attribute__((extern, used, artificial, visibility("hidden")))
__hidden_builtin_2(int x);

/* Prototype 3: Only visibility and extern */
int __attribute__((visibility("hidden"), extern))
__hidden_builtin_3(int x);

/* Prototype 4: With volatile-like behavior hint */
int __attribute__((visibility("hidden"), used, artificial, noinline))
__hidden_builtin_4(int x) __asm__("__hidden_builtin_4_internal");

/* ============================================
   TARGET-SPECIFIC BUILT-IN DECLARATIONS
   ============================================ */

#ifdef __i386__
/* x86 specific built-ins */
int __attribute__((visibility("hidden"), extern, used, artificial))
__builtin_ia32_rdtsc(void);

int __attribute__((visibility("hidden"), extern, used, artificial))
__builtin_ia32_crc32qi(int crc, char v);

void __attribute__((visibility("hidden"), extern, used, artificial))
__builtin_ia32_sfence(void);

#elif defined(__x86_64__)
/* x86_64 specific built-ins */
unsigned long long __attribute__((visibility("hidden"), extern, used, artificial))
__builtin_ia32_rdtsc(void);

unsigned int __attribute__((visibility("hidden"), extern, used, artificial))
__builtin_ia32_crc32qi(unsigned int crc, unsigned char v);

void __attribute__((visibility("hidden"), extern, used, artificial))
__builtin_ia32_mfence(void);

#elif defined(__arm__) || defined(__aarch64__)
/* ARM specific built-ins */
unsigned int __attribute__((visibility("hidden"), extern, used, artificial))
__builtin_arm_rbit(unsigned int x);

int __attribute__((visibility("hidden"), extern, used, artificial))
__builtin_arm_clz(int x);

#elif defined(__powerpc__) || defined(__ppc__) || defined(__PPC__)
/* PowerPC specific built-ins */
int __attribute__((visibility("hidden"), extern, used, artificial))
__builtin_ppc_popcntb(int x);

#else
/* Generic fallback - declare as weak to avoid linkage errors */
int __attribute__((visibility("hidden"), extern, used, artificial, weak))
__generic_hidden_builtin(int x);
#endif

/* ============================================
   FUNCTION POINTER ARRAY WITH VOLATILE STORAGE
   ============================================ */

/* Volatile function pointer array to prevent optimization */
typedef int (*func_ptr_t)(int);
static volatile func_ptr_t volatile_func_ptrs[8];

/* Opaque initialization that compiler can't analyze */
static void init_function_pointers(void) {
    int i;
    for (i = 0; i < 8; i++) {
        volatile_func_ptrs[i] = (func_ptr_t)((long)get_opaque_value() + i);
    }
}

/* ============================================
   MAIN FUNCTION WITH NON-OPTIMIZABLE LOGIC
   ============================================ */

int main(int argc, char **argv) {
    int result = 0;
    volatile func_ptr_t current_func = NULL;
    
    /* Use argv to create input-dependent behavior */
    if (argc > 1) {
        global_seed = atoi(argv[1]);
    } else {
        global_seed = 42;
    }
    
    init_function_pointers();
    
    /* Create a non-optimizable conditional based on input */
    int selector = get_opaque_value() & 0x7;
    
    /* Target-specific built-in usage with volatile pointer */
#ifdef __i386__
    if (selector == 0) {
        /* Take address of x86 built-in */
        current_func = (func_ptr_t)__builtin_ia32_rdtsc;
    } else if (selector == 1) {
        /* Call through asm to ensure processing */
        int (*volatile ptr)(int, char) = (int (*)(int, char))__builtin_ia32_crc32qi;
        result = ptr(0, 'A');
    }
#elif defined(__x86_64__)
    if (selector == 0) {
        current_func = (func_ptr_t)__builtin_ia32_rdtsc;
    } else if (selector == 1) {
        unsigned int (*volatile ptr)(unsigned int, unsigned char) = 
            (unsigned int (*)(unsigned int, unsigned char))__builtin_ia32_crc32qi;
        result = ptr(0, 'A');
    }
#elif defined(__arm__) || defined(__aarch64__)
    if (selector == 0) {
        current_func = (func_ptr_t)__builtin_arm_rbit;
    }
#endif
    
    /* Loop over function pointer array with opaque operations */
    for (int i = 0; i < 8; i++) {
        if (volatile_func_ptrs[i] != NULL) {
            /* Create comparison that can't be optimized away */
            if ((long)volatile_func_ptrs[i] == (long)current_func + i) {
                result ^= i;
            }
        }
    }
    
    /* Use inline asm to create optimization barrier */
    asm volatile("" : "+r" (result) : : "memory");
    
    /* Final opaque output */
    printf("Result: %d (seed: %d)\n", result, global_seed);
    
    return result & 0xFF;
}

/* ============================================
   ADDITIONAL DECLARATIONS TO INCREASE COVERAGE
   ============================================ */

/* Declare with asm name to trigger different paths */
extern int __attribute__((visibility("hidden"), extern, used, artificial))
__custom_builtin_impl(int) __asm__("__custom_builtin");

/* Multiple declarations with varying flags */
int __attribute__((visibility("hidden"), noinline, noclone))
decl_without_extern(int x) { return x + 1; }

int __attribute__((extern, visibility("hidden")))
extern_hidden_only(int x);

/* Array of function pointers declared with attributes */
static int (* __attribute__((visibility("hidden"))) 
    hidden_func_array[4])(int) = { NULL };

/* Constructor to ensure processing */
static void __attribute__((constructor, visibility("hidden")))
init_hidden_funcs(void) {
    hidden_func_array[0] = __hidden_builtin_1;
    hidden_func_array[1] = __hidden_builtin_2;
    hidden_func_array[2] = __hidden_builtin_3;
    hidden_func_array[3] = __hidden_builtin_4;
}
