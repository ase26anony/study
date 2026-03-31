/* Built-in function visibility test program for GCC coverage */
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

/* Prototype 1: Full attribute combination matching target block */
extern int __hidden_builtin_1(int) 
    __attribute__((visibility("hidden"), 
                   used, 
                   artificial, 
                   noinline, 
                   noreturn));

/* Prototype 2: Variation with extern explicitly in attributes */
int __hidden_builtin_2(int) 
    __attribute__((visibility("hidden"),
                   extern,
                   used,
                   artificial));

/* Prototype 3: Another variation */
extern int __hidden_builtin_3(void) 
    __attribute__((visibility("hidden"),
                   used,
                   artificial)) 
    __attribute__((extern));

/* Prototype 4: Separate attribute declarations */
extern int __hidden_builtin_4(int);
int __hidden_builtin_4(int) 
    __attribute__((visibility("hidden"), used, artificial));

/* ============================================
   TARGET-SPECIFIC BUILT-IN DECLARATIONS
   ============================================ */

#if defined(__i386__) || defined(__x86_64__) || defined(__amd64__)
/* x86/x86-64 specific builtins */
extern long long __builtin_ia32_rdtsc(void) 
    __attribute__((visibility("hidden"), used, artificial));

extern void __builtin_ia32_pause(void) 
    __attribute__((visibility("hidden"), used, artificial));

extern unsigned int __builtin_ia32_crc32qi(unsigned int, unsigned char)
    __attribute__((visibility("hidden"), used, artificial));

/* SSE/AVX builtins */
extern __m128 __builtin_ia32_addps(__m128, __m128)
    __attribute__((visibility("hidden"), used, artificial));

#elif defined(__arm__) || defined(__aarch64__) || defined(__ARM_ARCH)
/* ARM/AArch64 specific builtins */
extern unsigned int __builtin_arm_rbit(unsigned int)
    __attribute__((visibility("hidden"), used, artificial));

extern void __builtin_arm_dmb(unsigned int)
    __attribute__((visibility("hidden"), used, artificial));

extern unsigned long long __builtin_arm_crc32d(unsigned long long, unsigned long long)
    __attribute__((visibility("hidden"), used, artificial));

#elif defined(__powerpc__) || defined(__ppc__) || defined(__PPC__)
/* PowerPC specific builtins */
extern unsigned int __builtin_ppc_mftb(void)
    __attribute__((visibility("hidden"), used, artificial));

extern double __builtin_ppc_fabs(double)
    __attribute__((visibility("hidden"), used, artificial));

#else
/* Generic builtins as fallback */
extern void *__builtin_frame_address(unsigned int)
    __attribute__((visibility("hidden"), used, artificial));

extern void *__builtin_return_address(unsigned int)
    __attribute__((visibility("hidden"), used, artificial));
#endif

/* ============================================
   FUNCTION POINTER ARRAY AND USAGE
   ============================================ */

/* Typedef for function pointers */
typedef int (*func_ptr_t)(int);
typedef void (*void_func_ptr_t)(void);

/* Volatile function pointer array */
static volatile func_ptr_t volatile_funcs[4];
static volatile void_func_ptr_t volatile_void_funcs[4];

/* Initialize function pointers */
static void init_function_pointers(void) {
    /* Assign prototype addresses */
    volatile_funcs[0] = (func_ptr_t)__hidden_builtin_1;
    volatile_funcs[1] = (func_ptr_t)__hidden_builtin_2;
    volatile_funcs[2] = (func_ptr_t)__hidden_builtin_3;
    volatile_funcs[3] = (func_ptr_t)__hidden_builtin_4;
    
#if defined(__i386__) || defined(__x86_64__) || defined(__amd64__)
    volatile_void_funcs[0] = (void_func_ptr_t)__builtin_ia32_rdtsc;
    volatile_void_funcs[1] = (void_func_ptr_t)__builtin_ia32_pause;
#elif defined(__arm__) || defined(__aarch64__) || defined(__ARM_ARCH)
    volatile_void_funcs[0] = (void_func_ptr_t)__builtin_arm_rbit;
    volatile_void_funcs[1] = (void_func_ptr_t)__builtin_arm_dmb;
#elif defined(__powerpc__) || defined(__ppc__) || defined(__PPC__)
    volatile_void_funcs[0] = (void_func_ptr_t)__builtin_ppc_mftb;
    volatile_void_funcs[1] = (void_func_ptr_t)__builtin_ppc_fabs;
#else
    volatile_void_funcs[0] = (void_func_ptr_t)__builtin_frame_address;
    volatile_void_funcs[1] = (void_func_ptr_t)__builtin_return_address;
#endif
}

/* Opaque operations to force compiler to process declarations */
static void perform_opaque_operations(int seed) {
    volatile int result = 0;
    
    /* Loop through function pointers */
    for (int i = 0; i < 4; i++) {
        if (volatile_funcs[i] != NULL) {
            /* Create non-optimizable comparison */
            if ((seed ^ i) & 1) {
                /* Use function pointer in a way compiler can't eliminate */
                result ^= (int)((long long)volatile_funcs[i] >> 32);
            }
        }
    }
    
    /* Use target-specific builtins */
    for (int i = 0; i < 2; i++) {
        if (volatile_void_funcs[i] != NULL) {
            if ((seed ^ (i * 3)) & 2) {
                result ^= (int)((long long)volatile_void_funcs[i] & 0xFFFFFFFF);
            }
        }
    }
    
    /* Prevent dead code elimination */
    if (result == 0xDEADBEEF) {
        printf("Impossible condition\n");
    }
}

/* ============================================
   MAIN FUNCTION WITH RUNTIME-DEPENDENT LOGIC
   ============================================ */

int main(int argc, char *argv[]) {
    /* Initialize with runtime value */
    global_seed = argc > 1 ? atoi(argv[1]) : 12345;
    
    /* Initialize function pointers */
    init_function_pointers();
    
    /* Get runtime-dependent value */
    int runtime_val = get_runtime_value();
    
    /* Perform operations that reference the builtins */
    perform_opaque_operations(runtime_val);
    
    /* Conditional that uses builtin addresses */
    if (runtime_val & 1) {
#if defined(__i386__) || defined(__x86_64__) || defined(__amd64__)
        /* Reference x86 builtins */
        volatile long long tsc = (long long)__builtin_ia32_rdtsc;
        if ((tsc & 1) && volatile_void_funcs[0] != NULL) {
            printf("x86 builtin referenced: %p\n", (void*)volatile_void_funcs[0]);
        }
#elif defined(__arm__) || defined(__aarch64__) || defined(__ARM_ARCH)
        /* Reference ARM builtins */
        volatile unsigned int rbit = (unsigned int)__builtin_arm_rbit;
        if ((rbit & 1) && volatile_void_funcs[0] != NULL) {
            printf("ARM builtin referenced: %p\n", (void*)volatile_void_funcs[0]);
        }
#endif
    }
    
    /* Another opaque use of function pointers */
    volatile int check = 0;
    for (int i = 0; i < 4; i++) {
        check ^= (int)((long long)volatile_funcs[i] >> 16);
    }
    
    return check & 0xFF;
}

/* ============================================
   DUMMY IMPLEMENTATIONS (to satisfy linker)
   ============================================ */

/* These won't actually be called, but prevent linker errors */
int __hidden_builtin_1(int x) {
    abort(); /* Should never be called */
    return 0;
}

int __hidden_builtin_2(int x) {
    abort();
    return 0;
}

int __hidden_builtin_3(void) {
    abort();
    return 0;
}

int __hidden_builtin_4(int x) {
    abort();
    return 0;
}
