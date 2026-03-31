/* Built-in function visibility test program */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global volatile variable to prevent optimization */
volatile int global_seed = 0;

/* ============================================
   PROTOTYPES WITH VARIOUS ATTRIBUTE COMBINATIONS
   These should trigger built-in processing paths
   ============================================ */

/* Prototype 1: Full attribute set matching target block */
int __attribute__((visibility("hidden"), extern, used, artificial, noinline, noreturn))
__hidden_builtin_proto1(int x);

/* Prototype 2: Hidden visibility with extern */
int __attribute__((visibility("hidden"), extern))
__hidden_builtin_proto2(int x);

/* Prototype 3: Hidden with used and artificial */
int __attribute__((visibility("hidden"), used, artificial))
__hidden_builtin_proto3(int x);

/* Prototype 4: Just hidden visibility */
int __attribute__((visibility("hidden")))
__hidden_builtin_proto4(int x);

/* Prototype 5: Hidden with volatile linkage simulation */
int __attribute__((visibility("hidden"), noinline))
__hidden_builtin_proto5(int x);

/* ============================================
   TARGET-SPECIFIC BUILT-IN DECLARATIONS
   These will trigger target hooks
   ============================================ */

/* x86/x86_64 specific built-ins */
#if defined(__i386__) || defined(__x86_64__) || defined(__amd64__)

/* Declare actual x86 built-ins with hidden visibility */
int __attribute__((visibility("hidden"), used, artificial))
__builtin_ia32_rdtsc(void);

int __attribute__((visibility("hidden"), extern, used))
__builtin_ia32_crc32qi(int crc, char v);

void __attribute__((visibility("hidden"), artificial))
__builtin_ia32_pause(void);

unsigned int __attribute__((visibility("hidden"), extern, used, artificial))
__builtin_ia32_rdrand32_step(unsigned int *__ptr);

/* Additional x86 built-in with full attribute set */
int __attribute__((visibility("hidden"), extern, used, artificial, noinline))
__builtin_ia32_dummy_builtin(int x);

#endif

/* ARM specific built-ins */
#if defined(__arm__) || defined(__aarch64__) || defined(__ARM_ARCH)

int __attribute__((visibility("hidden"), extern, used, artificial))
__builtin_arm_rbit(int x);

unsigned int __attribute__((visibility("hidden"), used))
__builtin_arm_clz(unsigned int x);

void __attribute__((visibility("hidden"), artificial))
__builtin_arm_dmb(void);

#endif

/* PowerPC specific built-ins */
#if defined(__powerpc__) || defined(__ppc__) || defined(__PPC__)

int __attribute__((visibility("hidden"), extern, used, artificial))
__builtin_ppc_popcntb(int x);

unsigned int __attribute__((visibility("hidden"), used))
__builtin_ppc_mftb(void);

#endif

/* Generic fallback if no architecture-specific built-ins available */
#ifndef __has_builtin
#define __has_builtin(x) 0
#endif

/* ============================================
   VOLATILE FUNCTION POINTERS
   Prevent optimization of built-in references
   ============================================ */

/* Typedef for function pointer */
typedef int (*func_ptr_t)(int);

/* Array of volatile function pointers */
volatile func_ptr_t volatile_funcs[5];

/* Volatile pointer to hold built-in address */
volatile void *volatile_builtin_addr = NULL;

/* ============================================
   HELPER FUNCTIONS
   ============================================ */

/* Opaque function to prevent optimization */
static int __attribute__((noinline))
get_opaque_value(void) {
    return global_seed ^ 0xDEADBEEF;
}

/* Function that processes function pointers opaquely */
static void __attribute__((noinline, optimize("O0")))
process_function_pointers(void) {
    volatile int i;
    volatile func_ptr_t local_fp;
    
    /* Initialize array with various function pointers */
    for (i = 0; i < 5; i++) {
        volatile_funcs[i] = (func_ptr_t)&__hidden_builtin_proto1;
    }
    
    /* Opaque assignment to volatile pointer */
    local_fp = volatile_funcs[get_opaque_value() % 5];
    
    /* Store address in volatile location */
    volatile_builtin_addr = (void *)local_fp;
}

/* ============================================
   MAIN FUNCTION
   ============================================ */

int main(int argc, char *argv[]) {
    volatile int result = 0;
    volatile func_ptr_t current_func = NULL;
    
    /* Use argv to create input-dependent behavior */
    if (argc > 1) {
        global_seed = atoi(argv[1]);
    } else {
        global_seed = 12345;
    }
    
    /* Process function pointers to ensure declarations are used */
    process_function_pointers();
    
    /* ============================================
       TARGET-SPECIFIC BUILT-IN USAGE
       ============================================ */
    
#if defined(__i386__) || defined(__x86_64__) || defined(__amd64__)
    /* Use x86 built-ins with volatile pointers */
    {
        volatile void *addr1 = (void *)&__builtin_ia32_rdtsc;
        volatile void *addr2 = (void *)&__builtin_ia32_crc32qi;
        volatile void *addr3 = (void *)&__builtin_ia32_dummy_builtin;
        
        /* Non-optimizable comparison */
        if (addr1 != addr2) {
            current_func = (func_ptr_t)addr3;
        }
        
        /* Call through volatile pointer if condition is met */
        if (current_func && (global_seed % 2 == 0)) {
            result = current_func(global_seed);
        }
        
        /* Use the built-in directly in a way that can't be optimized away */
        volatile unsigned int rand_val;
        if (__builtin_ia32_rdrand32_step(&rand_val)) {
            result ^= rand_val;
        }
    }
#endif
    
#if defined(__arm__) || defined(__aarch64__) || defined(__ARM_ARCH)
    /* Use ARM built-ins */
    {
        volatile void *arm_addr = (void *)&__builtin_arm_rbit;
        volatile_builtin_addr = arm_addr;
        
        if (global_seed % 3 == 0) {
            result = __builtin_arm_rbit(global_seed);
        }
    }
#endif
    
#if defined(__powerpc__) || defined(__ppc__) || defined(__PPC__)
    /* Use PowerPC built-ins */
    {
        volatile void *ppc_addr = (void *)&__builtin_ppc_popcntb;
        if (volatile_builtin_addr != ppc_addr) {
            volatile_builtin_addr = ppc_addr;
        }
    }
#endif
    
    /* ============================================
       GENERIC BUILT-IN SIMULATION
       ============================================ */
    
    /* Array of function pointers to prototypes */
    volatile func_ptr_t proto_funcs[] = {
        (func_ptr_t)&__hidden_builtin_proto1,
        (func_ptr_t)&__hidden_builtin_proto2,
        (func_ptr_t)&__hidden_builtin_proto3,
        (func_ptr_t)&__hidden_builtin_proto4,
        (func_ptr_t)&__hidden_builtin_proto5,
        NULL
    };
    
    /* Loop through prototypes performing opaque operations */
    for (volatile int i = 0; i < 5; i++) {
        volatile void *current_addr = (void *)proto_funcs[i];
        
        /* Non-optimizable comparison chain */
        if (current_addr == volatile_builtin_addr) {
            result += i * 2;
        } else if (current_addr != NULL) {
            result += i;
        }
        
        /* Store address in volatile global */
        if (i == (global_seed % 5)) {
            volatile_builtin_addr = current_addr;
        }
    }
    
    /* Final opaque use of result */
    if (result != 0xDEADBEEF) {
        printf("Result: %d (seed: %d)\n", result, global_seed);
    }
    
    return result != 0 ? 0 : 1;
}

/* ============================================
   DUMMY IMPLEMENTATIONS (if not replaced by built-ins)
   ============================================ */

int __hidden_builtin_proto1(int x) {
    return x ^ 0x55AA55AA;
}

int __hidden_builtin_proto2(int x) {
    return x + 0x1000;
}

int __hidden_builtin_proto3(int x) {
    return x * 2;
}

int __hidden_builtin_proto4(int x) {
    return x / 2;
}

int __hidden_builtin_proto5(int x) {
    return x & 0xFF;
}
