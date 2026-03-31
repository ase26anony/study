/* Built-in function visibility test for targhooks.cc coverage */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global volatile variable to prevent optimization */
volatile int global_seed = 0;

/* Function to create runtime-dependent condition */
static int get_runtime_value(void) {
    return global_seed ^ (int)(__builtin_return_address(0) & 0xFF);
}

/* ============================================
   PROTOTYPES WITH VISIBILITY ATTRIBUTES
   ============================================ */

/* Prototype 1: Full attribute combination */
extern int __attribute__((visibility("hidden"), used, artificial, noinline))
    __hidden_builtin_1(int x) __asm__("__hidden_builtin_1");

/* Prototype 2: Hidden visibility with extern */
extern int __attribute__((visibility("hidden")))
    __hidden_builtin_2(int x, int y);

/* Prototype 3: Hidden + used + artificial */
int __attribute__((visibility("hidden"), used, artificial))
    __hidden_builtin_3(void) __asm__("__hidden_builtin_3");

/* Prototype 4: Just hidden visibility */
int __attribute__((visibility("hidden")))
    __hidden_builtin_4(long x);

/* Prototype 5: Hidden with volatile asm name */
extern int __attribute__((visibility("hidden"), used))
    __hidden_builtin_5(void) __asm__("__hidden_builtin_5");

/* ============================================
   TARGET-SPECIFIC BUILT-IN DECLARATIONS
   ============================================ */

#if defined(__i386__) || defined(__x86_64__)
/* x86/x86-64 specific built-ins */
extern long long __attribute__((visibility("hidden"), used, artificial))
    __builtin_ia32_rdtsc(void);

extern void __attribute__((visibility("hidden")))
    __builtin_ia32_sfence(void);

extern unsigned char __attribute__((visibility("hidden"), used))
    __builtin_ia32_addcarryx_u32(unsigned char, unsigned int, 
                                 unsigned int, unsigned int *);

extern int __attribute__((visibility("hidden"), artificial))
    __builtin_ia32_pmovmskb128(__v16qi);
#endif

#if defined(__arm__) || defined(__aarch64__)
/* ARM specific built-ins */
extern unsigned int __attribute__((visibility("hidden"), used, artificial))
    __builtin_arm_rbit(unsigned int);

extern void __attribute__((visibility("hidden")))
    __builtin_arm_dsb(unsigned int);

extern unsigned int __attribute__((visibility("hidden"), used))
    __builtin_arm_clz(unsigned int);
#endif

#if defined(__powerpc__) || defined(__ppc__) || defined(__PPC__)
/* PowerPC specific built-ins */
extern unsigned int __attribute__((visibility("hidden"), used, artificial))
    __builtin_ppc_mftb(void);

extern double __attribute__((visibility("hidden")))
    __builtin_ppc_fabs(double);
#endif

/* ============================================
   FUNCTION POINTER ARRAY WITH VOLATILE STORAGE
   ============================================ */

/* Volatile function pointer array to prevent optimization */
typedef int (*func_ptr_t)(void);
static volatile func_ptr_t volatile_funcs[8];

/* Opaque operations that compiler can't optimize away */
static void perform_opaque_operations(void) {
    volatile int i, j;
    volatile void* ptrs[4];
    
    /* Create some opaque pointer manipulations */
    for (i = 0; i < 4; i++) {
        ptrs[i] = (volatile void*)(&volatile_funcs[0] + i);
    }
    
    /* Opaque memory operations */
    for (j = 0; j < 2; j++) {
        for (i = 0; i < 4; i++) {
            /* This creates a data dependency the compiler can't analyze */
            *(volatile char*)ptrs[i] ^= (char)(get_runtime_value() + i + j);
        }
    }
}

/* ============================================
   MAIN FUNCTION WITH UNOPTIMIZABLE CONDITIONS
   ============================================ */

int main(int argc, char *argv[]) {
    /* Initialize global seed from argv if available */
    if (argc > 1) {
        global_seed = atoi(argv[1]);
    } else {
        global_seed = 12345;
    }
    
    /* Initialize volatile function pointer array */
    volatile_funcs[0] = (func_ptr_t)__hidden_builtin_1;
    volatile_funcs[1] = (func_ptr_t)__hidden_builtin_3;
    volatile_funcs[2] = (func_ptr_t)__hidden_builtin_5;
    
    /* Target-specific built-in assignments */
#if defined(__i386__) || defined(__x86_64__)
    volatile_funcs[3] = (func_ptr_t)__builtin_ia32_rdtsc;
    volatile_funcs[4] = (func_ptr_t)__builtin_ia32_sfence;
#elif defined(__arm__) || defined(__aarch64__)
    volatile_funcs[3] = (func_ptr_t)__builtin_arm_rbit;
    volatile_funcs[4] = (func_ptr_t)__builtin_arm_clz;
#elif defined(__powerpc__) || defined(__ppc__) || defined(__PPC__)
    volatile_funcs[3] = (func_ptr_t)__builtin_ppc_mftb;
    volatile_funcs[4] = (func_ptr_t)__builtin_ppc_fabs;
#endif
    
    /* Perform opaque operations to ensure declarations are processed */
    perform_opaque_operations();
    
    /* Create runtime-dependent condition that can't be optimized */
    int result = 0;
    volatile int condition = get_runtime_value() & 0x3;
    
    /* Unoptimizable switch on volatile condition */
    switch (condition) {
        case 0:
            /* Call through volatile function pointer */
            if (volatile_funcs[0] != NULL) {
                result = volatile_funcs[0](result);
            }
            break;
        case 1:
            /* Compare function addresses (can't be optimized) */
            if ((void*)volatile_funcs[1] != (void*)volatile_funcs[2]) {
                result = 1;
            }
            break;
        case 2:
            /* Use target-specific built-in if available */
#if defined(__i386__) || defined(__x86_64__)
            if (volatile_funcs[3] != NULL) {
                long long ts = ((long long (*)(void))volatile_funcs[3])();
                result = (int)(ts & 0xFFFFFFFF);
            }
#elif defined(__arm__) || defined(__aarch64__)
            if (volatile_funcs[3] != NULL) {
                unsigned int val = ((unsigned int (*)(unsigned int))volatile_funcs[3])(result);
                result = (int)val;
            }
#endif
            break;
        default:
            /* More opaque operations */
            for (int i = 0; i < 3; i++) {
                if (volatile_funcs[i] != NULL) {
                    result ^= (int)(long)volatile_funcs[i];
                }
            }
            break;
    }
    
    /* Additional loop to ensure all function pointers are referenced */
    for (int i = 0; i < 5; i++) {
        if (volatile_funcs[i] != NULL) {
            /* Create artificial data dependency */
            result += (int)((long)volatile_funcs[i] >> (i * 4));
        }
    }
    
    printf("Result: %d (seed: %d)\n", result, global_seed);
    return result & 0xFF;
}

/* ============================================
   DUMMY IMPLEMENTATIONS (if not built-in)
   ============================================ */

/* These provide implementations if the built-ins aren't available,
   but the compiler should recognize them as built-ins first */

int __hidden_builtin_1(int x) {
    return x ^ 0x55AA55AA;
}

int __hidden_builtin_2(int x, int y) {
    return x + y;
}

int __hidden_builtin_3(void) {
    return global_seed;
}

int __hidden_builtin_4(long x) {
    return (int)(x >> 32) ^ (int)x;
}

int __hidden_builtin_5(void) {
    static int counter = 0;
    return counter++;
}
