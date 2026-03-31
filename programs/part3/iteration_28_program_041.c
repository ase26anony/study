/* 
 * Program to trigger targhooks.cc uncovered lines 981-990
 * Compile with: gcc -O0 -march=native -fdump-tree-original -fvisibility=hidden
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global volatile variable to prevent optimization */
volatile int global_seed = 0;

/* 
 * Function prototypes with various attribute combinations
 * These mimic built-in declarations that should trigger the target hook
 */

/* Core prototype with all relevant attributes */
int __attribute__((visibility("hidden"), extern, used, artificial, noinline, noreturn))
__hidden_builtin_proto(int x);

/* Variant 1: hidden visibility with extern */
int __attribute__((visibility("hidden"), extern))
__hidden_extern_only(int x);

/* Variant 2: hidden visibility with used */
int __attribute__((visibility("hidden"), used))
__hidden_used_only(int x);

/* Variant 3: hidden visibility with artificial */
int __attribute__((visibility("hidden"), artificial))
__hidden_artificial_only(int x);

/* Variant 4: All flags except visibility specified */
int __attribute__((extern, used, artificial))
__all_except_visibility(int x);

/* Variant 5: Just hidden visibility */
int __attribute__((visibility("hidden")))
__just_hidden(int x);

/* Target-specific built-in declarations */
#ifdef __i386__

/* x86 specific built-ins */
int __attribute__((visibility("hidden"), extern, used, artificial))
__builtin_ia32_rdtsc(void);

int __attribute__((visibility("hidden"), extern, used, artificial))
__builtin_ia32_cpuid(int regs[4], int leaf);

void __attribute__((visibility("hidden"), extern, used, artificial))
__builtin_ia32_pause(void);

#elif defined(__x86_64__)

/* x86_64 specific built-ins */
int __attribute__((visibility("hidden"), extern, used, artificial))
__builtin_ia32_rdtsc(void);

long long __attribute__((visibility("hidden"), extern, used, artificial))
__builtin_ia32_rdtscp(unsigned int *aux);

void __attribute__((visibility("hidden"), extern, used, artificial))
__builtin_ia32_mfence(void);

#elif defined(__arm__) || defined(__aarch64__)

/* ARM specific built-ins */
unsigned int __attribute__((visibility("hidden"), extern, used, artificial))
__builtin_arm_rbit(unsigned int x);

void __attribute__((visibility("hidden"), extern, used, artificial))
__builtin_arm_dmb(unsigned int x);

int __attribute__((visibility("hidden"), extern, used, artificial))
__builtin_arm_clz(unsigned int x);

#elif defined(__powerpc__) || defined(__ppc__) || defined(__PPC__)

/* PowerPC specific built-ins */
unsigned int __attribute__((visibility("hidden"), extern, used, artificial))
__builtin_ppc_mftb(void);

int __attribute__((visibility("hidden"), extern, used, artificial))
__builtin_ppc_popcntb(unsigned int x);

#else

/* Generic fallback built-ins */
void __attribute__((visibility("hidden"), extern, used, artificial))
__builtin_trap(void);

void __attribute__((visibility("hidden"), extern, used, artificial))
__builtin_unreachable(void);

#endif

/* Array of function pointers to ensure processing */
typedef int (*func_ptr_t)(int);
volatile func_ptr_t func_array[10];

/* Opaque operation to prevent optimization */
static int opaque_operation(int x) {
    volatile int result = 0;
    for (int i = 0; i < 10; i++) {
        result ^= (x >> i) & 1;
    }
    return result;
}

int main(int argc, char *argv[]) {
    /* Use argv to create input-dependent behavior */
    int input = 0;
    if (argc > 1) {
        input = atoi(argv[1]);
    }
    
    /* Initialize global seed from input */
    global_seed = input;
    
    /* Volatile function pointer to prevent optimization */
    volatile func_ptr_t volatile_fp = NULL;
    
    /* Initialize function pointer array */
    for (int i = 0; i < 10; i++) {
        func_array[i] = NULL;
    }
    
    /* 
     * Assign built-in addresses based on architecture
     * This should trigger the target hook processing
     */
#ifdef __i386__
    volatile_fp = (func_ptr_t)__builtin_ia32_rdtsc;
    func_array[0] = (func_ptr_t)__builtin_ia32_rdtsc;
    func_array[1] = (func_ptr_t)__builtin_ia32_cpuid;
#elif defined(__x86_64__)
    volatile_fp = (func_ptr_t)__builtin_ia32_rdtsc;
    func_array[0] = (func_ptr_t)__builtin_ia32_rdtsc;
    func_array[1] = (func_ptr_t)__builtin_ia32_rdtscp;
#elif defined(__arm__) || defined(__aarch64__)
    volatile_fp = (func_ptr_t)__builtin_arm_rbit;
    func_array[0] = (func_ptr_t)__builtin_arm_rbit;
    func_array[1] = (func_ptr_t)__builtin_arm_clz;
#elif defined(__powerpc__) || defined(__ppc__) || defined(__PPC__)
    volatile_fp = (func_ptr_t)__builtin_ppc_mftb;
    func_array[0] = (func_ptr_t)__builtin_ppc_mftb;
    func_array[1] = (func_ptr_t)__builtin_ppc_popcntb;
#else
    volatile_fp = (func_ptr_t)__builtin_trap;
    func_array[0] = (func_ptr_t)__builtin_trap;
    func_array[1] = (func_ptr_t)__builtin_unreachable;
#endif
    
    /* Also assign prototype addresses */
    func_array[2] = __hidden_builtin_proto;
    func_array[3] = __hidden_extern_only;
    func_array[4] = __hidden_used_only;
    func_array[5] = __hidden_artificial_only;
    func_array[6] = __all_except_visibility;
    func_array[7] = __just_hidden;
    
    /* Create a non-optimizable conditional */
    int result = 0;
    if (global_seed != 0) {
        /* This branch uses the volatile function pointer */
        if (volatile_fp != NULL) {
            /* Simulate a call through the pointer */
            result = opaque_operation((int)(long)volatile_fp);
        }
    } else {
        /* Process all function pointers in the array */
        for (int i = 0; i < 8; i++) {
            if (func_array[i] != NULL) {
                result ^= opaque_operation((int)(long)func_array[i]);
            }
        }
    }
    
    /* Additional opaque operations to ensure processing */
    volatile int final_result = 0;
    for (int i = 0; i < 100; i++) {
        final_result ^= (result >> (i % 32)) & 1;
    }
    
    /* Use the result to prevent dead code elimination */
    if (final_result == 0x12345678) {
        /* This should never happen, but prevents optimization */
        printf("Impossible!\n");
    }
    
    return final_result & 1;
}

/* Dummy implementations to satisfy linker (won't be called) */
int __hidden_builtin_proto(int x) { return x; }
int __hidden_extern_only(int x) { return x + 1; }
int __hidden_used_only(int x) { return x + 2; }
int __hidden_artificial_only(int x) { return x + 3; }
int __all_except_visibility(int x) { return x + 4; }
int __just_hidden(int x) { return x + 5; }
