/* Built-in function visibility test program */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global volatile variable to prevent optimization */
volatile int global_seed = 0;

/* ==============================================
 * PROTOTYPES WITH VISIBILITY ATTRIBUTES
 * These mimic built-in declarations that should
 * trigger the target hook processing
 * ============================================== */

/* Prototype 1: Full attribute combination */
extern int __attribute__((visibility("hidden"), used, artificial, noinline))
    __hidden_builtin_1(int x);

/* Prototype 2: Hidden visibility with extern */
extern int __attribute__((visibility("hidden")))
    __hidden_builtin_2(int x, int y);

/* Prototype 3: Hidden + used + artificial */
extern int __attribute__((visibility("hidden"), used, artificial))
    __hidden_builtin_3(void);

/* Prototype 4: Just hidden visibility */
int __attribute__((visibility("hidden")))
    __hidden_builtin_4(long x);

/* Prototype 5: Hidden with volatile pointer return */
extern void* __attribute__((visibility("hidden"), used))
    __hidden_builtin_5(void* ptr);

/* ==============================================
 * TARGET-SPECIFIC BUILT-IN DECLARATIONS
 * Using actual GCC built-ins for various architectures
 * ============================================== */

/* x86/x86_64 specific built-ins */
#if defined(__i386__) || defined(__x86_64__) || defined(__i386) || defined(__amd64__)

/* Use various x86 intrinsic built-ins */
extern long long __attribute__((visibility("hidden"), used, artificial))
    __builtin_ia32_rdtsc(void);

extern void __attribute__((visibility("hidden"), used))
    __builtin_ia32_sfence(void);

extern unsigned int __attribute__((visibility("hidden"), artificial))
    __builtin_ia32_crc32qi(unsigned int, unsigned char);

extern int __attribute__((visibility("hidden"), used, artificial))
    __builtin_ia32_addsd(int, int);

/* Function to test x86 built-ins */
static void test_x86_builtins(volatile int seed) {
    volatile void* volatile_ptr = NULL;
    volatile long long (*volatile rdtsc_ptr)(void) = __builtin_ia32_rdtsc;
    volatile void (*volatile sfence_ptr)(void) = __builtin_ia32_sfence;
    
    if (seed & 1) {
        long long tsc = rdtsc_ptr();
        global_seed = (int)(tsc & 0xFFFFFFFF);
    }
    
    if (seed & 2) {
        sfence_ptr();
    }
    
    /* Use crc32 built-in */
    unsigned int crc = __builtin_ia32_crc32qi(0x12345678, (unsigned char)seed);
    global_seed ^= crc;
}

#endif

/* ARM specific built-ins */
#if defined(__arm__) || defined(__aarch64__) || defined(__thumb__)

extern unsigned int __attribute__((visibility("hidden"), used, artificial))
    __builtin_arm_rbit(unsigned int);

extern void __attribute__((visibility("hidden"), used))
    __builtin_arm_dmb(unsigned int);

extern int __attribute__((visibility("hidden"), artificial))
    __builtin_arm_clz(int);

/* Function to test ARM built-ins */
static void test_arm_builtins(volatile int seed) {
    volatile unsigned int (*volatile rbit_ptr)(unsigned int) = __builtin_arm_rbit;
    volatile void (*volatile dmb_ptr)(unsigned int) = __builtin_arm_dmb;
    
    if (seed & 1) {
        unsigned int reversed = rbit_ptr((unsigned int)seed);
        global_seed = reversed;
    }
    
    if (seed & 4) {
        dmb_ptr(0xF); /* Full system DMB */
    }
    
    /* Use clz built-in */
    int leading_zeros = __builtin_arm_clz(seed);
    global_seed += leading_zeros;
}

#endif

/* PowerPC specific built-ins */
#if defined(__powerpc__) || defined(__ppc__) || defined(__PPC__)

extern int __attribute__((visibility("hidden"), used, artificial))
    __builtin_ppc_popcntb(int);

extern long __attribute__((visibility("hidden"), used))
    __builtin_ppc_mftb(void);

/* Function to test PowerPC built-ins */
static void test_ppc_builtins(volatile int seed) {
    volatile long (*volatile mftb_ptr)(void) = __builtin_ppc_mftb;
    volatile int (*volatile popcnt_ptr)(int) = __builtin_ppc_popcntb;
    
    if (seed & 1) {
        long tb = mftb_ptr();
        global_seed = (int)(tb & 0xFFFF);
    }
    
    if (seed & 8) {
        int cnt = popcnt_ptr(seed);
        global_seed ^= cnt;
    }
}

#endif

/* Generic fallback if no architecture-specific built-ins are available */
#ifndef __i386__
#ifndef __x86_64__
#ifndef __arm__
#ifndef __aarch64__
#ifndef __powerpc__
#ifndef __ppc__

/* Use generic GCC built-ins with hidden visibility */
extern int __attribute__((visibility("hidden"), used, artificial))
    __builtin_popcount(unsigned int);

extern void* __attribute__((visibility("hidden"), used))
    __builtin_frame_address(unsigned int);

extern int __attribute__((visibility("hidden"), artificial))
    __builtin_ffs(int);

static void test_generic_builtins(volatile int seed) {
    volatile int (*volatile popcount_ptr)(unsigned int) = __builtin_popcount;
    volatile void* (*volatile frame_addr_ptr)(unsigned int) = __builtin_frame_address;
    
    if (seed & 1) {
        int popcnt = popcount_ptr((unsigned int)seed);
        global_seed = popcnt;
    }
    
    if (seed & 16) {
        void* frame = frame_addr_ptr(0);
        global_seed += (int)((long)frame & 0xFF);
    }
}

#endif
#endif
#endif
#endif
#endif
#endif

/* ==============================================
 * FUNCTION POINTER ARRAY FOR OPAQUE OPERATIONS
 * ============================================== */

/* Typedef for function pointers */
typedef int (*func_ptr_t)(int);

/* Array of function pointers to built-in-like functions */
static volatile func_ptr_t func_array[] = {
    (func_ptr_t)__hidden_builtin_1,
    (func_ptr_t)__hidden_builtin_2,
    (func_ptr_t)__hidden_builtin_3,
    (func_ptr_t)__hidden_builtin_4,
    NULL
};

/* ==============================================
 * MAIN FUNCTION WITH INPUT-DEPENDENT EXECUTION
 * ============================================== */

int main(int argc, char *argv[]) {
    volatile int seed = 0;
    
    /* Get seed from command line or use argc */
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = argc;
    }
    
    /* Make seed volatile to prevent compile-time optimization */
    volatile int volatile_seed = seed;
    
    /* ==============================================
     * TARGET-SPECIFIC BUILT-IN TESTING
     * This should trigger target hook processing
     * ============================================== */
    
#if defined(__i386__) || defined(__x86_64__) || defined(__i386) || defined(__amd64__)
    test_x86_builtins(volatile_seed);
#elif defined(__arm__) || defined(__aarch64__) || defined(__thumb__)
    test_arm_builtins(volatile_seed);
#elif defined(__powerpc__) || defined(__ppc__) || defined(__PPC__)
    test_ppc_builtins(volatile_seed);
#else
    test_generic_builtins(volatile_seed);
#endif
    
    /* ==============================================
     * OPAQUE LOOP OVER FUNCTION POINTER ARRAY
     * Forces compiler to process all declarations
     * ============================================== */
    
    for (int i = 0; i < 4; i++) {
        volatile func_ptr_t volatile_fp = func_array[i];
        
        /* Create input-dependent conditional */
        if (volatile_seed & (1 << i)) {
            if (volatile_fp != NULL) {
                /* This call may be optimized away, but the reference remains */
                int result = volatile_fp(volatile_seed + i);
                global_seed += result;
            }
        }
    }
    
    /* ==============================================
     * ADDITIONAL BUILT-IN ADDRESS COMPARISONS
     * Prevents optimization of built-in references
     * ============================================== */
    
    volatile void* volatile_addr;
    
#if defined(__i386__) || defined(__x86_64__)
    volatile_addr = (void*)__builtin_ia32_rdtsc;
#elif defined(__arm__) || defined(__aarch64__)
    volatile_addr = (void*)__builtin_arm_rbit;
#elif defined(__powerpc__) || defined(__ppc__)
    volatile_addr = (void*)__builtin_ppc_mftb;
#else
    volatile_addr = (void*)__builtin_popcount;
#endif
    
    /* Non-optimizable comparison */
    if ((long)volatile_addr > (long)&global_seed) {
        global_seed |= 0x80000000;
    }
    
    /* ==============================================
     * FINAL OUTPUT (prevents dead code elimination)
     * ============================================== */
    
    printf("Result: %d (seed was %d)\n", global_seed, seed);
    
    /* Use result in return value to prevent optimization */
    return (global_seed & 0xFF) == 0 ? 0 : 1;
}

/* ==============================================
 * DUMMY IMPLEMENTATIONS (to satisfy linker)
 * These won't be called if built-ins are used
 * ============================================== */

int __hidden_builtin_1(int x) {
    return x + 1;
}

int __hidden_builtin_2(int x, int y) {
    return x - y;
}

int __hidden_builtin_3(void) {
    return global_seed;
}

int __hidden_builtin_4(long x) {
    return (int)(x >> 32);
}

void* __hidden_builtin_5(void* ptr) {
    return (void*)((long)ptr + 1);
}
