/* 
 * Program to trigger targhooks.cc uncovered lines 981-990
 * Compile with: gcc -O0 -march=native -fdump-tree-original -fvisibility=hidden
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global volatile variable to prevent optimization */
volatile int global_seed = 0;

/* ============================================
 * SECTION 1: Built-in function prototypes with various attribute combinations
 * These should be processed by the compiler's built-in registration mechanism
 * ============================================ */

/* Prototype 1: Full attribute combination matching target block */
__attribute__((visibility("hidden"), extern, used, artificial, noinline, noreturn))
void __hidden_builtin_full(void);

/* Prototype 2: Partial attributes, may trigger different code paths */
__attribute__((visibility("hidden"), used, artificial))
int __hidden_builtin_int(int x);

/* Prototype 3: External linkage with hidden visibility */
__attribute__((visibility("hidden"), extern))
double __hidden_builtin_double(double a, double b);

/* Prototype 4: Used + artificial combination */
__attribute__((used, artificial, visibility("hidden")))
void* __hidden_builtin_ptr(void* p);

/* ============================================
 * SECTION 2: Target-specific built-in declarations
 * Using architecture-specific GCC built-ins
 * ============================================ */

#ifdef __x86_64__

/* x86-64 specific built-ins */
__attribute__((visibility("hidden"), extern, used, artificial))
unsigned long long __builtin_ia32_rdtsc(void);

__attribute__((visibility("hidden"), used, artificial))
int __builtin_ia32_addss(int a, int b);

__attribute__((visibility("hidden"), extern))
void __builtin_ia32_mfence(void);

#elif defined(__i386__)

/* i386 specific built-ins */
__attribute__((visibility("hidden"), extern, used, artificial))
unsigned long long __builtin_ia32_rdtsc(void);

__attribute__((visibility("hidden"), used, artificial))
int __builtin_ia32_paddb(int a, int b);

#elif defined(__arm__) || defined(__aarch64__)

/* ARM specific built-ins */
__attribute__((visibility("hidden"), extern, used, artificial))
unsigned int __builtin_arm_rbit(unsigned int x);

__attribute__((visibility("hidden"), used, artificial))
int __builtin_arm_clz(int x);

#elif defined(__powerpc__) || defined(__ppc__) || defined(__PPC__)

/* PowerPC specific built-ins */
__attribute__((visibility("hidden"), extern, used, artificial))
unsigned int __builtin_ppc_mftb(void);

#else

/* Generic fallback - declare dummy built-ins */
__attribute__((visibility("hidden"), extern, used, artificial))
int __dummy_builtin_1(void);

__attribute__((visibility("hidden"), used, artificial))
int __dummy_builtin_2(int x);

#endif

/* ============================================
 * SECTION 3: Function pointer declarations and usage
 * ============================================ */

/* Typedefs for function pointers */
typedef void (*volatile_func_ptr_void)(void);
typedef int (*volatile_func_ptr_int)(int);
typedef double (*volatile_func_ptr_double)(double, double);

/* Array of volatile function pointers to prevent optimization */
volatile_func_ptr_void volatile func_ptrs_void[4];
volatile_func_ptr_int volatile func_ptrs_int[4];

/* Opaque function to create non-optimizable conditions */
static int get_opaque_value(void) {
    return global_seed ^ (rand() & 0xFF);
}

/* ============================================
 * SECTION 4: Main function with built-in usage
 * ============================================ */

int main(int argc, char *argv[]) {
    /* Initialize seed from argv to create input-dependent behavior */
    if (argc > 1) {
        global_seed = atoi(argv[1]);
    } else {
        global_seed = 42;
    }
    
    srand(global_seed);
    
    /* ============================================
     * PART A: Take addresses of built-in-like functions
     * This should trigger built-in declaration processing
     * ============================================ */
    
    /* Store addresses in volatile pointers to prevent optimization */
    volatile void* addr1 = (void*)__hidden_builtin_full;
    volatile void* addr2 = (void*)__hidden_builtin_int;
    volatile void* addr3 = (void*)__hidden_builtin_double;
    
    /* Initialize function pointer arrays */
    func_ptrs_void[0] = __hidden_builtin_full;
    func_ptrs_int[0] = __hidden_builtin_int;
    
    /* ============================================
     * PART B: Target-specific built-in usage
     * ============================================ */
    
#ifdef __x86_64__
    func_ptrs_void[1] = (volatile_func_ptr_void)__builtin_ia32_rdtsc;
    func_ptrs_int[1] = __builtin_ia32_addss;
    func_ptrs_void[2] = __builtin_ia32_mfence;
    
    /* Create non-optimizable comparison */
    volatile unsigned long long ts1 = __builtin_ia32_rdtsc();
    if (get_opaque_value() > 100) {
        __builtin_ia32_mfence();
    }
    
#elif defined(__i386__)
    func_ptrs_void[1] = (volatile_func_ptr_void)__builtin_ia32_rdtsc;
    func_ptrs_int[1] = __builtin_ia32_paddb;
    
#elif defined(__arm__) || defined(__aarch64__)
    func_ptrs_int[1] = __builtin_arm_clz;
    func_ptrs_int[2] = (volatile_func_ptr_int)__builtin_arm_rbit;
    
#elif defined(__powerpc__) || defined(__ppc__) || defined(__PPC__)
    func_ptrs_int[1] = (volatile_func_ptr_int)__builtin_ppc_mftb;
    
#endif
    
    /* ============================================
     * PART C: Opaque operations with function pointers
     * This ensures compiler processes all declarations
     * ============================================ */
    
    int result = 0;
    
    /* Loop through function pointers with opaque index */
    for (int i = 0; i < 3; i++) {
        int idx = get_opaque_value() % 3;
        
        if (func_ptrs_int[idx] != NULL) {
            /* Create a call that can't be optimized away */
            if (idx == 0 && get_opaque_value() > 128) {
                /* This might trigger the built-in declaration path */
                result ^= (int)(long)func_ptrs_void[idx];
            }
        }
    }
    
    /* Final opaque use of addresses */
    if ((long)addr1 ^ (long)addr2 ^ (long)addr3) {
        result += global_seed;
    }
    
    /* Use result in a way that can't be optimized out */
    printf("Result: %d (seed: %d)\n", result, global_seed);
    
    return result & 1;
}

/* ============================================
 * SECTION 5: Dummy implementations (if needed for linking)
 * These won't be called if built-ins are properly recognized
 * ============================================ */

__attribute__((visibility("hidden"), noinline, noreturn))
void __hidden_builtin_full(void) {
    /* This should never be called if recognized as built-in */
    abort();
}

__attribute__((visibility("hidden"), noinline))
int __hidden_builtin_int(int x) {
    return x + 1;
}

__attribute__((visibility("hidden"), noinline))
double __hidden_builtin_double(double a, double b) {
    return a * b;
}

__attribute__((visibility("hidden"), noinline))
void* __hidden_builtin_ptr(void* p) {
    return p;
}
