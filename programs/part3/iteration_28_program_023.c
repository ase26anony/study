/* Built-in function visibility test program */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global volatile variable to prevent optimization */
volatile int global_seed = 0;

/* Function to create runtime-dependent condition */
static int get_runtime_value(void) {
    return global_seed & 1;
}

/* ============================================
   PROTOTYPES WITH VARIOUS ATTRIBUTE COMBINATIONS
   These should trigger built-in processing paths
   ============================================ */

/* Core prototype with all target attributes */
extern int __hidden_builtin_0(int) 
    __attribute__((visibility("hidden"), 
                   used, 
                   artificial, 
                   noinline, 
                   noreturn));

/* Variant with extern explicitly specified */
extern int __hidden_builtin_1(int, int) 
    __attribute__((visibility("hidden"), 
                   extern, 
                   used, 
                   artificial));

/* Another variant with different ordering */
int __hidden_builtin_2(void) 
    __attribute__((used, 
                   artificial, 
                   visibility("hidden"), 
                   extern));

/* Function pointer type for volatile storage */
typedef int (*func_ptr_t)(void);
typedef int (*func_ptr_int_t)(int);

/* ============================================
   TARGET-SPECIFIC BUILT-IN DECLARATIONS
   ============================================ */

/* x86/x86_64 specific built-ins */
#if defined(__i386__) || defined(__x86_64__) || defined(__amd64__)

/* Use various x86 built-ins that might be processed by target hooks */
extern long long __builtin_ia32_rdtsc(void) 
    __attribute__((visibility("hidden"), 
                   used, 
                   artificial));

extern void __builtin_ia32_sfence(void) 
    __attribute__((visibility("hidden"), 
                   extern, 
                   used, 
                   artificial));

extern unsigned char __builtin_ia32_loadups(const float*) 
    __attribute__((visibility("hidden"), 
                   used, 
                   artificial));

/* Additional x86 built-in with complex signature */
extern void __builtin_ia32_clflush(const void*) 
    __attribute__((visibility("hidden"), 
                   extern, 
                   used, 
                   artificial));

#endif

/* ARM specific built-ins */
#if defined(__arm__) || defined(__aarch64__) || defined(__ARM_ARCH)

extern unsigned int __builtin_arm_rbit(unsigned int) 
    __attribute__((visibility("hidden"), 
                   used, 
                   artificial));

extern void __builtin_arm_dmb(unsigned int) 
    __attribute__((visibility("hidden"), 
                   extern, 
                   used, 
                   artificial));

#endif

/* PowerPC specific built-ins */
#if defined(__powerpc__) || defined(__ppc__) || defined(__PPC__)

extern unsigned int __builtin_ppc_mftb(void) 
    __attribute__((visibility("hidden"), 
                   used, 
                   artificial));

#endif

/* ============================================
   MAIN FUNCTION WITH VOLATILE FUNCTION POINTERS
   ============================================ */

int main(int argc, char *argv[]) {
    /* Initialize volatile seed from argv if available */
    if (argc > 1) {
        global_seed = atoi(argv[1]);
    }
    
    /* Array of volatile function pointers to prevent optimization */
    volatile func_ptr_t volatile_funcs[4] = {0};
    volatile func_ptr_int_t volatile_int_funcs[4] = {0};
    
    /* Assign built-in addresses to volatile pointers */
    /* This should force the compiler to process the built-in declarations */
    
#if defined(__i386__) || defined(__x86_64__) || defined(__amd64__)
    volatile_funcs[0] = (func_ptr_t)__builtin_ia32_rdtsc;
    volatile_funcs[1] = (func_ptr_t)__builtin_ia32_sfence;
    volatile_int_funcs[0] = (func_ptr_int_t)__hidden_builtin_0;
#endif
    
#if defined(__arm__) || defined(__aarch64__) || defined(__ARM_ARCH)
    volatile_funcs[0] = (func_ptr_t)__builtin_arm_rbit;
    volatile_funcs[1] = (func_ptr_t)__builtin_arm_dmb;
#endif
    
    /* Create runtime-dependent condition that can't be optimized away */
    int condition = get_runtime_value();
    
    /* Opaque use of function pointers to ensure they're processed */
    for (int i = 0; i < 4; i++) {
        if (volatile_funcs[i] != 0 && condition) {
            /* This comparison forces the compiler to consider 
               the function addresses, but won't actually call them
               unless condition is true at runtime */
            if ((void*)volatile_funcs[i] == (void*)&main) {
                /* This should never happen, but prevents optimization */
                return 1;
            }
        }
    }
    
    /* Another opaque operation with the int-returning functions */
    int test_val = 42;
    for (int i = 0; i < 4; i++) {
        if (volatile_int_funcs[i] != 0) {
            /* Create a data dependency that can't be optimized */
            test_val ^= (int)(long)volatile_int_funcs[i];
        }
    }
    
    /* Use test_val to prevent dead code elimination */
    if (test_val == 0) {
        printf("Unexpected zero value\n");
    }
    
    /* Force a reference to all prototypes to ensure they're emitted */
    void* force_references[] = {
        (void*)__hidden_builtin_0,
        (void*)__hidden_builtin_1,
        (void*)__hidden_builtin_2,
#if defined(__i386__) || defined(__x86_64__) || defined(__amd64__)
        (void*)__builtin_ia32_rdtsc,
        (void*)__builtin_ia32_sfence,
        (void*)__builtin_ia32_loadups,
        (void*)__builtin_ia32_clflush,
#endif
#if defined(__arm__) || defined(__aarch64__) || defined(__ARM_ARCH)
        (void*)__builtin_arm_rbit,
        (void*)__builtin_arm_dmb,
#endif
#if defined(__powerpc__) || defined(__ppc__) || defined(__PPC__)
        (void*)__builtin_ppc_mftb,
#endif
        NULL
    };
    
    /* Opaque loop to use force_references */
    int sum = 0;
    for (int i = 0; force_references[i] != NULL; i++) {
        sum += ((long)force_references[i] & 1);
    }
    
    return sum & 1;
}

/* ============================================
   DUMMY IMPLEMENTATIONS (if needed)
   These might be needed if the compiler doesn't
   recognize them as built-ins
   ============================================ */

/* Weak implementations that won't be used if built-ins are recognized */
int __hidden_builtin_0(int x) 
    __attribute__((weak, visibility("hidden"), used, artificial)) {
    return x + 1;
}

int __hidden_builtin_1(int x, int y) 
    __attribute__((weak, visibility("hidden"), extern, used, artificial)) {
    return x + y;
}

int __hidden_builtin_2(void) 
    __attribute__((weak, used, artificial, visibility("hidden"), extern)) {
    return 42;
}
