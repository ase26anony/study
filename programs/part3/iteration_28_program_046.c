/* 
 * Program to trigger uncovered lines in targhooks.cc (lines 981-990)
 * Compile with: gcc -O0 -march=native -fdump-tree-original -fvisibility=hidden
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global volatile variable to prevent optimization */
volatile int global_seed = 0;

/* 
 * Function prototypes with various attribute combinations to trigger
 * different paths in built-in function processing
 */

/* Prototype 1: Full set of attributes matching target block */
__attribute__((visibility("hidden"), extern, used, artificial, noinline, noreturn))
void __hidden_builtin_1(void);

/* Prototype 2: Similar but without noreturn */
__attribute__((visibility("hidden"), extern, used, artificial, noinline))
int __hidden_builtin_2(int x);

/* Prototype 3: With volatile added */
__attribute__((visibility("hidden"), extern, used, artificial, noinline))
volatile int __hidden_builtin_3(volatile int x);

/* Prototype 4: With const attribute */
__attribute__((visibility("hidden"), extern, used, artificial, noinline, const))
int __hidden_builtin_4(void);

/* 
 * Target-specific built-in declarations
 * These will be processed by TARGET_BUILTIN_DECL or similar hooks
 */

#ifdef __i386__

/* x86-specific built-ins */
__attribute__((visibility("hidden"), extern, used, artificial))
int __builtin_ia32_emms(void);

__attribute__((visibility("hidden"), extern, used, artificial))
void __builtin_ia32_sfence(void);

__attribute__((visibility("hidden"), extern, used, artificial))
unsigned int __builtin_ia32_rdtsc(void);

#elif defined(__x86_64__)

/* x86_64-specific built-ins */
__attribute__((visibility("hidden"), extern, used, artificial))
void __builtin_ia32_lfence(void);

__attribute__((visibility("hidden"), extern, used, artificial))
void __builtin_ia32_mfence(void);

__attribute__((visibility("hidden"), extern, used, artificial))
__int128 __builtin_ia32_addcarryx_u64(unsigned char, unsigned long long,
                                      unsigned long long, unsigned long long *);

#elif defined(__arm__) || defined(__aarch64__)

/* ARM-specific built-ins */
__attribute__((visibility("hidden"), extern, used, artificial))
unsigned int __builtin_arm_rbit(unsigned int);

__attribute__((visibility("hidden"), extern, used, artificial))
void __builtin_arm_dmb(unsigned int);

__attribute__((visibility("hidden"), extern, used, artificial))
int __builtin_arm_clz(int);

#elif defined(__powerpc__) || defined(__ppc__) || defined(__PPC__)

/* PowerPC-specific built-ins */
__attribute__((visibility("hidden"), extern, used, artificial))
unsigned int __builtin_ppc_mftb(void);

__attribute__((visibility("hidden"), extern, used, artificial))
void __builtin_ppc_sync(void);

__attribute__((visibility("hidden"), extern, used, artificial))
int __builtin_ppc_popcntb(int);

#else

/* Generic fallback built-ins */
__attribute__((visibility("hidden"), extern, used, artificial))
void *__builtin_return_address(unsigned int level);

__attribute__((visibility("hidden"), extern, used, artificial))
void __builtin_unreachable(void);

__attribute__((visibility("hidden"), extern, used, artificial))
int __builtin_clz(unsigned int);

#endif

/* Array of volatile function pointers to prevent optimization */
typedef void (*volatile_func_ptr)(void);
typedef int (*volatile_int_func_ptr)(int);

/* Opaque operations that the compiler can't optimize away */
static void perform_opaque_operations(void) {
    volatile_func_ptr volatile_fp_array[4];
    volatile_int_func_ptr volatile_int_fp_array[4];
    
    /* Initialize with addresses of our prototypes */
    volatile_fp_array[0] = (volatile_func_ptr)__hidden_builtin_1;
    volatile_int_fp_array[0] = (volatile_int_func_ptr)__hidden_builtin_2;
    volatile_int_fp_array[1] = (volatile_int_func_ptr)__hidden_builtin_3;
    volatile_int_fp_array[2] = (volatile_int_func_ptr)__hidden_builtin_4;
    
    /* Target-specific built-ins */
#ifdef __i386__
    volatile_fp_array[1] = (volatile_func_ptr)__builtin_ia32_emms;
    volatile_fp_array[2] = (volatile_func_ptr)__builtin_ia32_sfence;
#elif defined(__x86_64__)
    volatile_fp_array[1] = (volatile_func_ptr)__builtin_ia32_lfence;
    volatile_fp_array[2] = (volatile_func_ptr)__builtin_ia32_mfence;
#elif defined(__arm__) || defined(__aarch64__)
    volatile_int_fp_array[3] = (volatile_int_func_ptr)__builtin_arm_clz;
#endif
    
    /* Force compiler to process these declarations */
    for (int i = 0; i < 4; i++) {
        if (volatile_fp_array[i]) {
            /* Create a barrier that compiler can't optimize */
            asm volatile("" : : "r"(volatile_fp_array[i]) : "memory");
        }
    }
}

/* Dummy implementations to satisfy linker (won't actually be called) */
void __hidden_builtin_1(void) {
    /* Should be noreturn, but we need to return for compilation */
    abort();
}

int __hidden_builtin_2(int x) {
    return x + global_seed;
}

volatile int __hidden_builtin_3(volatile int x) {
    return x ^ global_seed;
}

int __hidden_builtin_4(void) {
    return global_seed;
}

int main(int argc, char *argv[]) {
    /* Use argv to create input-dependent behavior */
    int input_value = 0;
    if (argc > 1) {
        input_value = atoi(argv[1]);
    }
    
    /* Volatile function pointer to prevent optimization */
    int (*volatile target_builtin)(int) = NULL;
    
    /* Choose which built-in to use based on input (unpredictable at compile time) */
    if (input_value % 3 == 0) {
        target_builtin = __hidden_builtin_2;
    } else if (input_value % 3 == 1) {
        target_builtin = __hidden_builtin_3;
    } else {
        target_builtin = __hidden_builtin_4;
    }
    
    /* Also include target-specific built-ins in the decision tree */
#ifdef __i386__
    if (input_value % 5 == 0) {
        /* Take address of x86 built-in */
        void (*volatile x86_builtin)(void) = __builtin_ia32_emms;
        asm volatile("" : : "r"(x86_builtin) : "memory");
    }
#elif defined(__x86_64__)
    if (input_value % 7 == 0) {
        void (*volatile x64_builtin)(void) = __builtin_ia32_lfence;
        asm volatile("" : : "r"(x64_builtin) : "memory");
    }
#endif
    
    /* Perform opaque operations to ensure declarations are processed */
    perform_opaque_operations();
    
    /* Create a conditional that can't be resolved at compile time */
    volatile int result = 0;
    if (target_builtin != NULL) {
        result = target_builtin(input_value);
    }
    
    /* Use result in a way that can't be optimized away */
    printf("Result: %d (seed: %d)\n", result, global_seed);
    
    /* Compare function pointers in a non-optimizable way */
    int (*volatile fp1)(int) = __hidden_builtin_2;
    int (*volatile fp2)(int) = __hidden_builtin_3;
    
    if (fp1 != fp2) {
        global_seed = 1;
    }
    
    return global_seed;
}

/* Additional declarations to increase coverage probability */
__attribute__((visibility("hidden"), extern, used, artificial, constructor(101)))
static void init_hidden_builtins(void) {
    /* This constructor should force early processing of declarations */
    global_seed = 42;
}

/* Another set of declarations with slightly different attributes */
__attribute__((visibility("hidden"), used, noinline))
static int secondary_hidden_func(int x) {
    return x * 2;
}

__attribute__((visibility("hidden"), extern, artificial, noreturn))
void external_hidden_noreturn(void) {
    while(1);
}
