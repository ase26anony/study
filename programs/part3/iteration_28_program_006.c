/* Built-in function visibility test program */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global volatile variable to prevent optimization */
volatile int global_seed = 0;

/* ============================================
   DECLARATION OF PROTOTYPES WITH TARGET ATTRIBUTES
   ============================================ */

/* Prototype 1: Full attribute combination */
extern int __hidden_builtin_1(int) 
    __attribute__((visibility("hidden"), 
                   used, 
                   artificial, 
                   noinline, 
                   noreturn));

/* Prototype 2: Varying attribute combinations */
extern void __hidden_builtin_2(void) 
    __attribute__((visibility("hidden"), 
                   used));

/* Prototype 3: Different ordering */
__attribute__((visibility("hidden"), 
               artificial, 
               extern))
int __hidden_builtin_3(float);

/* Prototype 4: Minimal attributes */
extern int __hidden_builtin_4(void) 
    __attribute__((visibility("hidden")));

/* ============================================
   TARGET-SPECIFIC BUILT-IN DECLARATIONS
   ============================================ */

#ifdef __i386__
/* x86 specific built-ins */
extern int __builtin_ia32_rdtsc(void) 
    __attribute__((visibility("hidden"), 
                   used, 
                   artificial));
extern void __builtin_ia32_sfence(void) 
    __attribute__((visibility("hidden")));
extern unsigned char __builtin_ia32_loadups(float*) 
    __attribute__((visibility("hidden"), 
                   used));
#endif

#ifdef __x86_64__
/* x86_64 specific built-ins */
extern long long __builtin_ia32_rdtsc(void) 
    __attribute__((visibility("hidden"), 
                   used, 
                   artificial));
extern void __builtin_ia32_clflush(const void*) 
    __attribute__((visibility("hidden")));
extern void __builtin_ia32_mfence(void) 
    __attribute__((visibility("hidden"), 
                   used, 
                   artificial));
#endif

#ifdef __arm__
/* ARM specific built-ins */
extern unsigned int __builtin_arm_rbit(unsigned int) 
    __attribute__((visibility("hidden"), 
                   used, 
                   artificial));
extern void __builtin_arm_dmb(unsigned int) 
    __attribute__((visibility("hidden")));
#endif

#ifdef __aarch64__
/* AArch64 specific built-ins */
extern unsigned long long __builtin_aarch64_rbitll(unsigned long long) 
    __attribute__((visibility("hidden"), 
                   used, 
                   artificial));
extern void __builtin_aarch64_yield(void) 
    __attribute__((visibility("hidden")));
#endif

#ifdef __powerpc__
/* PowerPC specific built-ins */
extern unsigned int __builtin_ppc_mftb(void) 
    __attribute__((visibility("hidden"), 
                   used, 
                   artificial));
extern void __builtin_ppc_sync(void) 
    __attribute__((visibility("hidden")));
#endif

/* ============================================
   FUNCTION POINTER ARRAY WITH VOLATILE STORAGE
   ============================================ */

/* Array of volatile function pointers to prevent optimization */
typedef void (*func_ptr_t)(void);
volatile func_ptr_t volatile_funcs[10];

/* Opaque function to create non-optimizable conditions */
static int get_opaque_value(void) {
    volatile int x = global_seed;
    return (x & 1) ? 1 : 0;
}

/* ============================================
   MAIN FUNCTION WITH BUILT-IN USAGE
   ============================================ */

int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Use argv to create input-dependent condition */
    int use_builtin = (argc > 1 && argv[1][0] != '\0') ? 1 : 0;
    
    /* Initialize volatile function pointer array */
    volatile_funcs[0] = (func_ptr_t)__hidden_builtin_1;
    volatile_funcs[1] = (func_ptr_t)__hidden_builtin_2;
    volatile_funcs[2] = (func_ptr_t)__hidden_builtin_3;
    volatile_funcs[3] = (func_ptr_t)__hidden_builtin_4;
    
    /* Target-specific built-in assignments */
#ifdef __i386__
    volatile_funcs[4] = (func_ptr_t)__builtin_ia32_rdtsc;
    volatile_funcs[5] = (func_ptr_t)__builtin_ia32_sfence;
#endif
    
#ifdef __x86_64__
    volatile_funcs[4] = (func_ptr_t)__builtin_ia32_rdtsc;
    volatile_funcs[5] = (func_ptr_t)__builtin_ia32_mfence;
#endif
    
#ifdef __arm__
    volatile_funcs[4] = (func_ptr_t)__builtin_arm_rbit;
    volatile_funcs[5] = (func_ptr_t)__builtin_arm_dmb;
#endif
    
    /* Loop to process function pointers (prevents dead code elimination) */
    for (int i = 0; i < 6; i++) {
        if (volatile_funcs[i] != NULL) {
            /* Create non-optimizable comparison */
            if (get_opaque_value() || use_builtin) {
                /* This comparison cannot be resolved at compile time */
                if ((unsigned long)volatile_funcs[i] != 0xDEADBEEF) {
                    result++;
                }
            }
        }
    }
    
    /* Additional conditional that might trigger built-in usage */
    volatile int *volatile_ptr = &global_seed;
    if (*volatile_ptr || use_builtin) {
        /* This creates a data dependency that prevents optimization */
#ifdef __i386__
        if (__builtin_ia32_rdtsc) {
            result += 2;
        }
#endif
        
#ifdef __x86_64__
        if (__builtin_ia32_rdtsc) {
            result += 2;
        }
#endif
    }
    
    /* Force compiler to consider all built-in declarations */
    asm volatile("" : : "r"(__hidden_builtin_1));
    asm volatile("" : : "r"(__hidden_builtin_2));
    asm volatile("" : : "r"(__hidden_builtin_3));
    asm volatile("" : : "r"(__hidden_builtin_4));
    
    printf("Result: %d\n", result);
    return result > 0 ? 0 : 1;
}

/* ============================================
   DUMMY IMPLEMENTATIONS (to satisfy linker)
   ============================================ */

/* These won't actually be called, but prevent linker errors */
int __hidden_builtin_1(int x) {
    abort(); /* noreturn attribute */
}

void __hidden_builtin_2(void) {
    global_seed = 1;
}

int __hidden_builtin_3(float f) {
    return (int)f;
}

int __hidden_builtin_4(void) {
    return global_seed;
}
