/* Built-in function visibility test program */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global volatile variable to prevent optimization */
volatile int global_seed = 0;

/* ============================================
   PROTOTYPES WITH VISIBILITY ATTRIBUTES
   ============================================ */

/* Prototype 1: Full attribute combination */
extern int __attribute__((visibility("hidden"), used, artificial))
    __hidden_builtin_1(int x) __asm__("__hidden_builtin_1");

/* Prototype 2: Hidden visibility with external linkage */
extern int __attribute__((visibility("hidden")))
    __hidden_builtin_2(int x, int y);

/* Prototype 3: Hidden + used + artificial */
int __attribute__((visibility("hidden"), used, artificial, noinline))
    __hidden_builtin_3(void);

/* Prototype 4: Just hidden visibility */
int __attribute__((visibility("hidden")))
    __hidden_builtin_4(int *ptr);

/* ============================================
   TARGET-SPECIFIC BUILT-IN DECLARATIONS
   ============================================ */

/* x86/x86_64 specific built-ins */
#if defined(__i386__) || defined(__x86_64__) || defined(__i686__)

/* Use actual GCC x86 built-ins */
extern int __attribute__((visibility("hidden"), used, artificial))
    __builtin_ia32_rdtsc(void);

extern void __attribute__((visibility("hidden")))
    __builtin_ia32_pause(void);

extern unsigned int __attribute__((visibility("hidden"), used))
    __builtin_ia32_crc32qi(unsigned int, unsigned char);

/* Declare with asm name to ensure external linkage */
extern long long __attribute__((visibility("hidden"), artificial))
    __builtin_ia32_rdpmc(int ecx) __asm__("__builtin_ia32_rdpmc");

#endif

/* ARM specific built-ins */
#if defined(__arm__) || defined(__aarch64__) || defined(__ARM_ARCH)

extern unsigned int __attribute__((visibility("hidden"), used, artificial))
    __builtin_arm_rbit(unsigned int);

extern void __attribute__((visibility("hidden")))
    __builtin_arm_dmb(unsigned int);

extern int __attribute__((visibility("hidden"), artificial))
    __builtin_arm_clz(int);

#endif

/* PowerPC specific built-ins */
#if defined(__powerpc__) || defined(__ppc__) || defined(__PPC__)

extern int __attribute__((visibility("hidden"), used, artificial))
    __builtin_ppc_mftb(void);

extern void __attribute__((visibility("hidden")))
    __builtin_ppc_sync(void);

#endif

/* Generic built-in with hidden visibility */
extern void * __attribute__((visibility("hidden"), used, artificial))
    __builtin_frame_address(unsigned int level);

extern void * __attribute__((visibility("hidden")))
    __builtin_return_address(unsigned int level);

/* ============================================
   FUNCTION POINTER ARRAY FOR OPAQUE OPERATIONS
   ============================================ */

/* Volatile function pointer type */
typedef int (*volatile_func_ptr_t)(int);

/* Array of function pointers for opaque operations */
static volatile_func_ptr_t volatile func_ptrs[4];

/* ============================================
   HELPER FUNCTIONS
   ============================================ */

/* Opaque use of function pointers to ensure processing */
static void __attribute__((noinline))
process_function_pointers(int seed) {
    volatile int result = 0;
    
    /* Initialize function pointers based on seed */
    for (int i = 0; i < 4; i++) {
        /* Opaque initialization that compiler can't optimize away */
        if ((seed >> i) & 1) {
            /* Use target-specific built-ins when available */
            #if defined(__i386__) || defined(__x86_64__)
            if (i == 0) {
                /* Take address of built-in through asm */
                func_ptrs[i] = (volatile_func_ptr_t)__builtin_ia32_rdtsc;
            }
            #endif
            
            /* Use generic built-in */
            if (i == 1) {
                func_ptrs[i] = (volatile_func_ptr_t)__builtin_frame_address;
            }
        }
    }
    
    /* Opaque use of function pointers */
    for (int i = 0; i < 4; i++) {
        if (func_ptrs[i]) {
            /* Create non-optimizable comparison */
            if ((unsigned long)func_ptrs[i] != (unsigned long)&process_function_pointers) {
                result += i;
            }
        }
    }
    
    /* Volatile write to prevent dead code elimination */
    *(volatile int *)&global_seed = result;
}

/* ============================================
   MAIN FUNCTION
   ============================================ */

int main(int argc, char *argv[]) {
    int seed = 0;
    
    /* Get seed from command line or use default */
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = 42;
    }
    
    /* Volatile copy to prevent optimization */
    volatile int vseed = seed;
    
    /* Initialize global seed */
    global_seed = vseed;
    
    /* Process function pointers with opaque operations */
    process_function_pointers(vseed);
    
    /* Additional opaque operations with built-ins */
    
    #if defined(__i386__) || defined(__x86_64__)
    /* Use x86 built-ins */
    {
        volatile unsigned long long tsc1, tsc2;
        tsc1 = __builtin_ia32_rdtsc();
        
        /* Opaque computation */
        for (int i = 0; i < (vseed & 0xFF); i++) {
            __builtin_ia32_pause();
        }
        
        tsc2 = __builtin_ia32_rdtsc();
        
        /* Non-optimizable comparison */
        if (tsc2 > tsc1) {
            global_seed += (tsc2 - tsc1) & 0xFF;
        }
    }
    #endif
    
    #if defined(__arm__) || defined(__aarch64__)
    /* Use ARM built-ins */
    {
        volatile unsigned int val = vseed;
        volatile unsigned int reversed = __builtin_arm_rbit(val);
        
        if (reversed != val) {
            global_seed += __builtin_arm_clz(val);
        }
    }
    #endif
    
    /* Use generic built-ins with hidden visibility */
    {
        volatile void *frame_addr = __builtin_frame_address(0);
        volatile void *return_addr = __builtin_return_address(0);
        
        /* Opaque comparison */
        if (frame_addr != return_addr) {
            global_seed += (unsigned long)frame_addr & 0xF;
        }
    }
    
    /* Final opaque result */
    int result = global_seed & 0xFF;
    
    /* Use result in a way that can't be optimized away */
    if (result != seed) {
        printf("Result: %d (seed was %d)\n", result, seed);
    } else {
        printf("Result equals seed: %d\n", result);
    }
    
    return result;
}

/* ============================================
   ADDITIONAL DECLARATIONS TO INCREASE COVERAGE
   ============================================ */

/* More prototypes with different attribute combinations */
extern int __attribute__((visibility("hidden"), noinline, noclone))
    __extra_builtin_1(int x);

extern void __attribute__((visibility("hidden"), constructor))
    __init_hidden_builtin(void);

extern void __attribute__((visibility("hidden"), destructor))
    __cleanup_hidden_builtin(void);

/* Weak symbol with hidden visibility */
extern int __attribute__((visibility("hidden"), weak))
    __weak_hidden_builtin(int x);

/* Pure function with hidden visibility */
extern int __attribute__((visibility("hidden"), pure))
    __pure_hidden_builtin(int x);

/* Const function with hidden visibility */
extern int __attribute__((visibility("hidden"), const))
    __const_hidden_builtin(int x);
