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
   PROTOTYPES WITH VARIOUS ATTRIBUTE COMBINATIONS
   ============================================ */

/* Core prototype with all target attributes */
extern int __attribute__((visibility("hidden"), used, artificial, noinline))
__hidden_builtin_proto(int x) __asm__("__hidden_builtin_impl");

/* Variant 1: hidden visibility with extern */
extern int __attribute__((visibility("hidden")))
__hidden_extern_only(int x);

/* Variant 2: hidden with used */
int __attribute__((visibility("hidden"), used))
__hidden_used_only(int x);

/* Variant 3: hidden with artificial */
int __attribute__((visibility("hidden"), artificial))
__hidden_artificial_only(int x);

/* Variant 4: All flags except visibility specified */
extern int __attribute__((used, artificial, noinline))
__all_except_visibility(int x);

/* Variant 5: Just hidden visibility */
int __attribute__((visibility("hidden")))
__just_hidden(int x);

/* ============================================
   TARGET-SPECIFIC BUILT-IN DECLARATIONS
   ============================================ */

/* x86/x86_64 specific built-ins */
#if defined(__i386__) || defined(__x86_64__) || defined(_M_IX86) || defined(_M_X64)

/* Declare actual x86 built-ins with hidden visibility */
extern long long __attribute__((visibility("hidden"), used, artificial))
__builtin_ia32_rdtsc(void);

extern void __attribute__((visibility("hidden"), used, artificial))
__builtin_ia32_pause(void);

extern unsigned int __attribute__((visibility("hidden"), used, artificial))
__builtin_ia32_crc32qi(unsigned int, unsigned char);

/* x86 SIMD built-in */
extern int __attribute__((visibility("hidden"), used, artificial))
__builtin_ia32_addss(int, int);

#endif

/* ARM specific built-ins */
#if defined(__arm__) || defined(__aarch64__) || defined(_M_ARM) || defined(_M_ARM64)

/* ARM CRC built-in */
extern unsigned int __attribute__((visibility("hidden"), used, artificial))
__builtin_arm_crc32b(unsigned int, unsigned char);

/* ARM DSP built-in */
extern int __attribute__((visibility("hidden"), used, artificial))
__builtin_arm_qadd(int, int);

/* ARM barrier built-in */
extern void __attribute__((visibility("hidden"), used, artificial))
__builtin_arm_dmb(unsigned int);

#endif

/* PowerPC specific built-ins */
#if defined(__powerpc__) || defined(__ppc__) || defined(__PPC__) || defined(_M_PPC)

/* PPC altivec built-in */
extern int __attribute__((visibility("hidden"), used, artificial))
__builtin_altivec_vaddubm(int, int);

/* PPC mtfsf built-in */
extern void __attribute__((visibility("hidden"), used, artificial))
__builtin_mtfsf(unsigned int, double);

#endif

/* Generic built-in fallbacks */
extern void __attribute__((visibility("hidden"), used, artificial))
__builtin_trap(void);

extern int __attribute__((visibility("hidden"), used, artificial))
__builtin_popcount(unsigned int);

extern void __attribute__((visibility("hidden"), used, artificial))
__builtin_prefetch(const void *, ...);

/* ============================================
   VOLATILE FUNCTION POINTERS
   ============================================ */

/* Typedef for function pointers */
typedef int (*func_ptr_t)(int);
typedef void (*void_func_ptr_t)(void);

/* Volatile function pointers to prevent optimization */
volatile func_ptr_t volatile_fp1 = 0;
volatile func_ptr_t volatile_fp2 = 0;
volatile void_func_ptr_t volatile_void_fp = 0;

/* Array of function pointers for iteration */
static func_ptr_t func_array[] = {
    (func_ptr_t)__hidden_builtin_proto,
    (func_ptr_t)__hidden_extern_only,
    (func_ptr_t)__hidden_used_only,
    (func_ptr_t)__hidden_artificial_only,
    (func_ptr_t)__all_except_visibility,
    (func_ptr_t)__just_hidden,
    (func_ptr_t)__builtin_popcount,
    0 /* Sentinel */
};

/* ============================================
   MAIN FUNCTION WITH OPAQUE OPERATIONS
   ============================================ */

int main(int argc, char *argv[]) {
    int result = 0;
    int i;
    
    /* Initialize global_seed from argv if available */
    if (argc > 1) {
        global_seed = atoi(argv[1]);
    } else {
        global_seed = get_runtime_value();
    }
    
    /* ============================================
       TARGET-SPECIFIC BUILT-IN USAGE
       ============================================ */
    
#if defined(__i386__) || defined(__x86_64__) || defined(_M_IX86) || defined(_M_X64)
    /* Use x86 built-ins */
    volatile_fp1 = (func_ptr_t)__builtin_ia32_rdtsc;
    volatile_void_fp = (void_func_ptr_t)__builtin_ia32_pause;
    
    /* Create runtime-dependent condition */
    if (global_seed & 1) {
        volatile_void_fp();
    }
    
    /* Use CRC built-in */
    result = __builtin_ia32_crc32qi(result, (unsigned char)global_seed);
    
#elif defined(__arm__) || defined(__aarch64__) || defined(_M_ARM) || defined(_M_ARM64)
    /* Use ARM built-ins */
    volatile_fp1 = (func_ptr_t)__builtin_arm_crc32b;
    volatile_fp2 = (func_ptr_t)__builtin_arm_qadd;
    
    result = __builtin_arm_crc32b(result, (unsigned char)global_seed);
    
#elif defined(__powerpc__) || defined(__ppc__) || defined(__PPC__) || defined(_M_PPC)
    /* Use PowerPC built-ins */
    volatile_fp1 = (func_ptr_t)__builtin_altivec_vaddubm;
    
    result = __builtin_altivec_vaddubm(result, global_seed);
    
#endif
    
    /* ============================================
       GENERIC BUILT-IN USAGE
       ============================================ */
    
    /* Use generic built-ins */
    volatile_fp1 = (func_ptr_t)__builtin_popcount;
    volatile_void_fp = (void_func_ptr_t)__builtin_trap;
    
    /* Prefetch based on runtime value */
    __builtin_prefetch(&global_seed, 0, 3);
    
    /* Create complex condition that can't be optimized away */
    int condition = get_runtime_value();
    
    /* Non-optimizable comparison of function pointers */
    if ((condition & 0xF) == 0) {
        /* Call through volatile function pointer */
        if (volatile_fp1) {
            result = volatile_fp1(condition);
        }
    } else if ((condition & 0xF) == 1) {
        /* Compare function pointer addresses */
        if (volatile_fp1 != volatile_fp2) {
            result = __builtin_popcount(condition);
        }
    } else if ((condition & 0xF) == 2) {
        /* Potential trap call */
        if (volatile_void_fp && (condition & 0x100)) {
            volatile_void_fp();
        }
    }
    
    /* ============================================
       ITERATE OVER FUNCTION POINTER ARRAY
       ============================================ */
    
    /* Loop through function pointer array with opaque operations */
    for (i = 0; func_array[i] != 0; i++) {
        /* Create opaque predicate */
        int opaque = (global_seed >> i) & 1;
        
        if (opaque) {
            /* Store address in volatile to force processing */
            volatile func_ptr_t temp = func_array[i];
            (void)temp;
            
            /* Use in computation that can't be eliminated */
            result ^= (int)((long long)func_array[i] & 0xFFFF);
        }
    }
    
    /* ============================================
       FINAL OPAQUE OUTPUT
       ============================================ */
    
    /* Final computation that depends on everything */
    result = result ^ global_seed ^ argc;
    
    /* Use result in a way that can't be optimized away */
    volatile int final_result = result;
    
    /* Return value based on runtime computation */
    return final_result & 0xFF;
}

/* ============================================
   DUMMY IMPLEMENTATIONS (if not built-in)
   ============================================ */

/* Provide weak implementations in case built-ins aren't available */
int __attribute__((weak, visibility("hidden")))
__hidden_builtin_proto(int x) {
    return x ^ 0x55;
}

int __attribute__((weak, visibility("hidden")))
__hidden_extern_only(int x) {
    return x + 1;
}

int __attribute__((weak, visibility("hidden")))
__hidden_used_only(int x) {
    return x * 2;
}

int __attribute__((weak, visibility("hidden")))
__hidden_artificial_only(int x) {
    return x / 2;
}

int __attribute__((weak))
__all_except_visibility(int x) {
    return x | 0xFF;
}

int __attribute__((weak, visibility("hidden")))
__just_hidden(int x) {
    return x & 0x7F;
}
