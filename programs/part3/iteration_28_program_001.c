/* 
 * Program to trigger built-in function declaration with hidden visibility
 * Target: targhooks.cc lines 981-990
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global volatile variable to prevent optimization */
volatile int global_seed = 0;

/* ============================================
 * PROTOTYPES WITH VISIBILITY ATTRIBUTES
 * ============================================ */

/* Prototype 1: Full attribute combination */
extern int __attribute__((visibility("hidden"), used, artificial, noinline))
__hidden_builtin_1(int x);

/* Prototype 2: Varying attribute combinations */
extern int __attribute__((visibility("hidden"), used))
__hidden_builtin_2(int x);

/* Prototype 3: Different order */
int __attribute__((used, artificial, visibility("hidden"), extern))
__hidden_builtin_3(int x);

/* Prototype 4: Minimal but with visibility specified */
extern int __attribute__((visibility("hidden")))
__hidden_builtin_4(int x);

/* ============================================
 * TARGET-SPECIFIC BUILT-IN DECLARATIONS
 * ============================================ */

#ifdef __i386__
/* x86 specific built-ins */
extern int __attribute__((visibility("hidden"), used, artificial))
__builtin_ia32_rdtsc(void);

extern void __attribute__((visibility("hidden"), used))
__builtin_ia32_pause(void);

extern int __attribute__((visibility("hidden"), artificial))
__builtin_ia32_add(int a, int b);

#elif defined(__x86_64__)
/* x86_64 specific built-ins */
extern long long __attribute__((visibility("hidden"), used, artificial))
__builtin_ia32_rdtsc(void);

extern void __attribute__((visibility("hidden"), used))
__builtin_ia32_pause(void);

extern int __attribute__((visibility("hidden"), artificial))
__builtin_popcountll(unsigned long long x);

#elif defined(__arm__) || defined(__aarch64__)
/* ARM specific built-ins */
extern int __attribute__((visibility("hidden"), used, artificial))
__builtin_arm_rbit(int x);

extern void __attribute__((visibility("hidden"), used))
__builtin_arm_dmb(void);

extern int __attribute__((visibility("hidden"), artificial))
__builtin_arm_clz(int x);

#else
/* Generic fallback built-ins */
extern int __attribute__((visibility("hidden"), used, artificial))
__builtin_abs(int x);

extern int __attribute__((visibility("hidden"), used))
__builtin_popcount(unsigned int x);

#endif

/* ============================================
 * VOLATILE FUNCTION POINTERS
 * ============================================ */

/* Array of volatile function pointers */
typedef int (*func_ptr_t)(int);
volatile func_ptr_t volatile_funcs[8];

/* Volatile comparison function pointer */
volatile func_ptr_t volatile_compare = NULL;

/* ============================================
 * HELPER FUNCTIONS
 * ============================================ */

/* Opaque function to prevent optimization */
static int __attribute__((noinline))
get_opaque_value(void) {
    return global_seed ^ 0x12345678;
}

/* Function to create non-optimizable condition */
static int __attribute__((noinline, used))
create_condition(int seed) {
    volatile int local_volatile = seed;
    return local_volatile & 0xF;
}

/* ============================================
 * MAIN FUNCTION
 * ============================================ */

int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Initialize global seed from argv if available */
    if (argc > 1) {
        global_seed = atoi(argv[1]);
    } else {
        global_seed = 42;
    }
    
    /* ============================================
     * INITIALIZE VOLATILE FUNCTION POINTERS
     * ============================================ */
    
    /* Initialize with target-specific built-ins */
#ifdef __i386__
    volatile_funcs[0] = (func_ptr_t)__builtin_ia32_add;
    volatile_funcs[1] = (func_ptr_t)__builtin_ia32_rdtsc;
#elif defined(__x86_64__)
    volatile_funcs[0] = (func_ptr_t)__builtin_popcountll;
    volatile_funcs[1] = (func_ptr_t)__builtin_ia32_rdtsc;
#elif defined(__arm__) || defined(__aarch64__)
    volatile_funcs[0] = (func_ptr_t)__builtin_arm_clz;
    volatile_funcs[1] = (func_ptr_t)__builtin_arm_rbit;
#else
    volatile_funcs[0] = (func_ptr_t)__builtin_abs;
    volatile_funcs[1] = (func_ptr_t)__builtin_popcount;
#endif
    
    /* Initialize with our hidden prototypes */
    volatile_funcs[2] = __hidden_builtin_1;
    volatile_funcs[3] = __hidden_builtin_2;
    volatile_funcs[4] = __hidden_builtin_3;
    volatile_funcs[5] = __hidden_builtin_4;
    
    /* Set volatile comparison pointer */
    volatile_compare = volatile_funcs[create_condition(global_seed) % 6];
    
    /* ============================================
     * NON-OPTIMIZABLE CONDITIONAL LOGIC
     * ============================================ */
    
    /* Create input-dependent condition */
    int condition = create_condition(global_seed);
    
    /* Complex conditional that can't be optimized away */
    for (int i = 0; i < 6; i++) {
        if ((condition >> i) & 1) {
            /* Compare function pointers - forces compiler to process declarations */
            if (volatile_funcs[i] == volatile_compare) {
                result += i * 100;
            }
            
            /* Opaque operation using function pointer */
            int (*temp_func)(int) = (int (*)(int))volatile_funcs[i];
            
            /* Call through pointer if condition is right */
            if (i == (condition & 3)) {
                /* Use get_opaque_value to prevent constant folding */
                int arg = get_opaque_value() & 0xFF;
                
                /* Store result to prevent dead code elimination */
                volatile int temp_result = 0;
                if (temp_func) {
                    /* In real scenario, we'd call, but for built-ins we just reference */
                    temp_result = arg + i;
                }
                result += temp_result;
            }
        }
    }
    
    /* ============================================
     * ADDITIONAL DECLARATIONS TO FORCE PROCESSING
     * ============================================ */
    
    /* Declare more built-ins with different attribute combinations */
    {
        /* Force processing by taking addresses */
        void *addr_array[] = {
            (void*)__hidden_builtin_1,
            (void*)__hidden_builtin_2,
            (void*)__hidden_builtin_3,
            (void*)__hidden_builtin_4,
#ifdef __i386__
            (void*)__builtin_ia32_rdtsc,
            (void*)__builtin_ia32_pause,
#elif defined(__x86_64__)
            (void*)__builtin_ia32_rdtsc,
            (void*)__builtin_ia32_pause,
#elif defined(__arm__) || defined(__aarch64__)
            (void*)__builtin_arm_rbit,
            (void*)__builtin_arm_dmb,
#endif
        };
        
        /* Opaque use of addresses */
        for (size_t j = 0; j < sizeof(addr_array)/sizeof(addr_array[0]); j++) {
            result += ((long)addr_array[j] & 0xF);
        }
    }
    
    printf("Result: %d (Seed: %d)\n", result, global_seed);
    return result & 0x7F; /* Return non-zero to prevent optimization */
}

/* ============================================
 * DUMMY IMPLEMENTATIONS (if not built-in)
 * ============================================ */

/* These provide implementations if the compiler doesn't treat them as built-ins */
int __attribute__((visibility("hidden"), used, artificial, noinline))
__hidden_builtin_1(int x) {
    return x + 1;
}

int __attribute__((visibility("hidden"), used))
__hidden_builtin_2(int x) {
    return x * 2;
}

int __attribute__((used, artificial, visibility("hidden"), extern))
__hidden_builtin_3(int x) {
    return x ^ 0xAA;
}

int __attribute__((visibility("hidden")))
__hidden_builtin_4(int x) {
    return -x;
}
