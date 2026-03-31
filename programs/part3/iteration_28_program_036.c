/* Built-in function visibility test program */
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
 * PROTOTYPES WITH VARIOUS ATTRIBUTE COMBINATIONS
 * These should trigger the target hook processing
 * ============================================ */

/* Core prototype with all relevant attributes */
extern int __hidden_builtin_0(int) 
    __attribute__((visibility("hidden"), used, artificial, noinline));

/* Variant with explicit extern */
extern int __hidden_builtin_1(int, int) 
    __attribute__((visibility("hidden"), extern, used));

/* Variant without artificial but with other flags */
extern int __hidden_builtin_2(void) 
    __attribute__((visibility("hidden"), used, noinline));

/* Variant with only visibility specified */
extern int __hidden_builtin_3(int) 
    __attribute__((visibility("hidden")));

/* ============================================
 * TARGET-SPECIFIC BUILT-IN DECLARATIONS
 * Using actual GCC built-ins for different architectures
 * ============================================ */

#ifdef __i386__
/* x86 specific built-ins */
extern long long __builtin_ia32_rdtsc(void) 
    __attribute__((visibility("hidden"), used, artificial));
    
extern void __builtin_ia32_sfence(void) 
    __attribute__((visibility("hidden"), used));

extern int __builtin_ia32_addcarryx_u32(unsigned char, unsigned int, 
                                        unsigned int, unsigned int *)
    __attribute__((visibility("hidden"), used, artificial));
#endif

#ifdef __x86_64__
/* x86_64 specific built-ins */
extern void __builtin_ia32_lfence(void) 
    __attribute__((visibility("hidden"), used, artificial));
    
extern unsigned long long __builtin_ia32_rdtscp(unsigned int *) 
    __attribute__((visibility("hidden"), used));
#endif

#ifdef __arm__
/* ARM specific built-ins */
extern unsigned int __builtin_arm_rbit(unsigned int) 
    __attribute__((visibility("hidden"), used, artificial));
    
extern void __builtin_arm_dsb(unsigned int) 
    __attribute__((visibility("hidden"), used));
#endif

#ifdef __aarch64__
/* AArch64 specific built-ins */
extern unsigned long long __builtin_aarch64_rdtsc(void) 
    __attribute__((visibility("hidden"), used, artificial));
#endif

#ifdef __powerpc__
/* PowerPC specific built-ins */
extern unsigned int __builtin_ppc_mftb(void) 
    __attribute__((visibility("hidden"), used, artificial));
#endif

/* ============================================
 * VOLATILE FUNCTION POINTERS
 * To prevent optimization and ensure processing
 * ============================================ */

/* Typedef for function pointers */
typedef int (*func_ptr_t)(int);
typedef void (*void_func_ptr_t)(void);

/* Volatile function pointer array */
static volatile func_ptr_t volatile_funcs[4];
static volatile void_func_ptr_t volatile_void_funcs[4];

/* ============================================
 * MAIN FUNCTION WITH OPAQUE OPERATIONS
 * ============================================ */

int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Initialize global seed from argv if available */
    if (argc > 1) {
        global_seed = atoi(argv[1]);
    } else {
        global_seed = get_runtime_value();
    }
    
    /* ============================================
     * ASSIGN BUILT-INS TO VOLATILE POINTERS
     * This forces the compiler to process the declarations
     * ============================================ */
    
    /* Assign our prototype functions */
    volatile_funcs[0] = (func_ptr_t)__hidden_builtin_0;
    volatile_funcs[1] = (func_ptr_t)__hidden_builtin_1;
    volatile_funcs[2] = (func_ptr_t)__hidden_builtin_2;
    volatile_funcs[3] = (func_ptr_t)__hidden_builtin_3;
    
    /* Assign architecture-specific built-ins */
#ifdef __i386__
    volatile_void_funcs[0] = (void_func_ptr_t)__builtin_ia32_rdtsc;
    volatile_void_funcs[1] = (void_func_ptr_t)__builtin_ia32_sfence;
#endif
    
#ifdef __x86_64__
    volatile_void_funcs[0] = (void_func_ptr_t)__builtin_ia32_lfence;
#endif
    
#ifdef __arm__
    volatile_void_funcs[0] = (void_func_ptr_t)__builtin_arm_rbit;
    volatile_void_funcs[1] = (void_func_ptr_t)__builtin_arm_dsb;
#endif
    
    /* ============================================
     * OPAQUE OPERATIONS WITH FUNCTION POINTERS
     * These cannot be optimized away
     * ============================================ */
    
    /* Create runtime-dependent index */
    int idx = global_seed % 4;
    
    /* Opaque comparison that compiler cannot resolve */
    if ((volatile_funcs[idx] != (func_ptr_t)0) && 
        (global_seed & 1)) {
        /* Call through volatile pointer */
        result = volatile_funcs[idx](global_seed);
    }
    
    /* Loop with opaque operations */
    for (int i = 0; i < 4; i++) {
        /* Force evaluation of function addresses */
        volatile int dummy = (int)(long)volatile_funcs[i];
        
        /* Opaque condition based on function address bits */
        if ((dummy & 0xF) == (global_seed & 0xF)) {
            result ^= dummy;
        }
    }
    
    /* ============================================
     * USE ARCHITECTURE-SPECIFIC BUILT-INS
     * ============================================ */
    
#ifdef __i386__
    /* Use x86 built-ins with opaque conditions */
    if (global_seed & 0x10) {
        unsigned int carry_out;
        unsigned char carry_in = (global_seed >> 8) & 1;
        unsigned int a = global_seed;
        unsigned int b = global_seed ^ 0x55AA55AA;
        
        __builtin_ia32_addcarryx_u32(carry_in, a, b, &carry_out);
        result += carry_out;
    }
#endif
    
#ifdef __arm__
    /* Use ARM built-ins */
    if (global_seed & 0x20) {
        unsigned int reversed = __builtin_arm_rbit(global_seed);
        result ^= reversed;
    }
#endif
    
    /* Final opaque computation */
    result = (result ^ global_seed) & 0xFF;
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d (seed: %d)\n", result, global_seed);
    
    return result & 1;
}

/* ============================================
 * DUMMY IMPLEMENTATIONS (if not built-in)
 * These provide definitions if the built-ins aren't available
 * ============================================ */

/* Weak attributes allow these to be overridden by built-ins */
int __hidden_builtin_0(int x) 
    __attribute__((weak, visibility("hidden"), used, artificial));
int __hidden_builtin_0(int x) {
    return x ^ 0x12345678;
}

int __hidden_builtin_1(int x, int y) 
    __attribute__((weak, visibility("hidden"), used));
int __hidden_builtin_1(int x, int y) {
    return x + y;
}

int __hidden_builtin_2(void) 
    __attribute__((weak, visibility("hidden"), used));
int __hidden_builtin_2(void) {
    return global_seed;
}

int __hidden_builtin_3(int x) 
    __attribute__((weak, visibility("hidden")));
int __hidden_builtin_3(int x) {
    return x * 2;
}
