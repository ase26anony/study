/* 
 * Program to trigger uncovered lines in targhooks.cc (lines 981-990)
 * Compile with: gcc -O0 -march=native -fdump-tree-original -fvisibility=hidden
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global volatile variable to prevent optimization */
volatile int global_volatile_counter = 0;

/* 
 * Function prototypes with various attribute combinations
 * These mimic built-in declarations that should trigger the target hook
 */

/* Prototype 1: Full set of attributes matching the uncovered lines */
int __attribute__((visibility("hidden"), extern, used, artificial, noinline, noclone))
hidden_builtin_proto1(int x);

/* Prototype 2: Visibility specified with different combination */
int __attribute__((visibility("hidden"), used, noinline))
hidden_builtin_proto2(int x);

/* Prototype 3: External with hidden visibility */
extern int __attribute__((visibility("hidden"), artificial))
hidden_builtin_proto3(int x);

/* Prototype 4: Used with hidden visibility */
int __attribute__((visibility("hidden"), used, noinline, noreturn))
hidden_builtin_proto4(void);

/* Target-specific built-in declarations based on architecture */
#ifdef __i386__

/* x86-specific built-ins */
int __attribute__((visibility("hidden"), extern, used, artificial))
__builtin_ia32_rdtsc(void);

int __attribute__((visibility("hidden"), extern, used, artificial))
__builtin_ia32_crc32qi(int crc, char v);

void __attribute__((visibility("hidden"), extern, used, artificial))
__builtin_ia32_pause(void);

#elif defined(__x86_64__)

/* x86_64-specific built-ins */
long long __attribute__((visibility("hidden"), extern, used, artificial))
__builtin_ia32_rdtsc(void);

unsigned int __attribute__((visibility("hidden"), extern, used, artificial))
__builtin_ia32_crc32qi(unsigned int crc, unsigned char v);

void __attribute__((visibility("hidden"), extern, used, artificial))
__builtin_ia32_pause(void);

#elif defined(__arm__)

/* ARM-specific built-ins */
unsigned int __attribute__((visibility("hidden"), extern, used, artificial))
__builtin_arm_rbit(unsigned int x);

int __attribute__((visibility("hidden"), extern, used, artificial))
__builtin_arm_clz(int x);

#elif defined(__aarch64__)

/* AArch64-specific built-ins */
unsigned long __attribute__((visibility("hidden"), extern, used, artificial))
__builtin_aarch64_rbit(unsigned long x);

int __attribute__((visibility("hidden"), extern, used, artificial))
__builtin_aarch64_clz(int x);

#else

/* Generic fallback - declare some GCC built-ins with hidden visibility */
int __attribute__((visibility("hidden"), extern, used, artificial))
__builtin_popcount(unsigned int x);

int __attribute__((visibility("hidden"), extern, used, artificial))
__builtin_clz(unsigned int x);

void __attribute__((visibility("hidden"), extern, used, artificial))
__builtin_unreachable(void);

#endif

/* Array of function pointers to ensure processing */
typedef int (*func_ptr_t)(int);
volatile func_ptr_t func_array[4];

/* Opaque operation to prevent optimization */
static int opaque_operation(int x) {
    volatile int result = x;
    for (int i = 0; i < 10; i++) {
        result ^= (result << 1) | (result >> 31);
    }
    return result;
}

int main(int argc, char *argv[]) {
    /* Use argv to create input-dependent behavior */
    int input_value = argc > 1 ? atoi(argv[1]) : 42;
    volatile int should_call = (input_value % 2) == 0;
    
    /* Initialize function pointer array */
    func_array[0] = (func_ptr_t)hidden_builtin_proto1;
    func_array[1] = (func_ptr_t)hidden_builtin_proto2;
    func_array[2] = (func_ptr_t)hidden_builtin_proto3;
    func_array[3] = (func_ptr_t)hidden_builtin_proto4;
    
    /* Volatile function pointer to prevent optimization */
    volatile func_ptr_t volatile_fp = NULL;
    int result = 0;
    
    /* Architecture-specific built-in usage */
#ifdef __i386__ || defined(__x86_64__)
    /* Use x86 built-ins */
    if (should_call) {
        volatile_fp = (func_ptr_t)__builtin_ia32_rdtsc;
        /* Create non-optimizable comparison */
        if ((void*)volatile_fp != (void*)&main) {
            __builtin_ia32_pause();
            result = __builtin_ia32_crc32qi(0, (char)input_value);
        }
    }
#elif defined(__arm__) || defined(__aarch64__)
    /* Use ARM built-ins */
    if (should_call) {
        volatile_fp = (func_ptr_t)__builtin_arm_clz;
        if ((void*)volatile_fp != (void*)&main) {
            result = __builtin_arm_clz(input_value);
        }
    }
#else
    /* Use generic built-ins */
    if (should_call) {
        volatile_fp = (func_ptr_t)__builtin_popcount;
        if ((void*)volatile_fp != (void*)&main) {
            result = __builtin_popcount(input_value);
            __builtin_unreachable(); /* This might trigger special handling */
        }
    }
#endif
    
    /* Loop through function pointers to ensure they're processed */
    for (int i = 0; i < 4; i++) {
        if (func_array[i] != NULL) {
            /* Opaque operation to prevent dead code elimination */
            global_volatile_counter += opaque_operation(i);
        }
    }
    
    /* Another volatile check to ensure built-in addresses are used */
    volatile int dummy_check = 0;
    if ((void*)volatile_fp == (void*)&hidden_builtin_proto1) {
        dummy_check = 1;
    }
    
    /* Mix results to prevent optimization */
    result ^= global_volatile_counter ^ dummy_check ^ input_value;
    
    printf("Result: %d\n", result);
    return result & 0xFF;
}

/* 
 * Dummy implementations to satisfy references (though built-ins
 * should be handled by compiler)
 */
int hidden_builtin_proto1(int x) {
    return x + 1;
}

int hidden_builtin_proto2(int x) {
    return x * 2;
}

int hidden_builtin_proto3(int x) {
    return x - 1;
}

int hidden_builtin_proto4(void) {
    return 0;
}
