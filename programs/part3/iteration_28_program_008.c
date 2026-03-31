/* Built-in visibility test program for targhooks.cc coverage */
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
   DECLARATION OF PROTOTYPES WITH TARGET ATTRIBUTES
   These should trigger the builtin_function_ext_scope path
   ============================================ */

/* Prototype 1: Full attribute combination */
extern int __attribute__((visibility("hidden"), extern, used, artificial))
    __hidden_builtin_1(int x);

/* Prototype 2: Hidden visibility with used */
extern int __attribute__((visibility("hidden"), used))
    __hidden_builtin_2(int x, int y);

/* Prototype 3: Hidden with artificial */
extern int __attribute__((visibility("hidden"), artificial))
    __hidden_builtin_3(void);

/* Prototype 4: Just hidden visibility */
extern int __attribute__((visibility("hidden")))
    __hidden_builtin_4(int x);

/* Prototype 5: Hidden with no throw */
extern int __attribute__((visibility("hidden"), nothrow))
    __hidden_builtin_5(int x);

/* ============================================
   TARGET-SPECIFIC BUILT-IN DECLARATIONS
   These use actual GCC built-ins for different architectures
   ============================================ */

/* x86/x86_64 specific built-ins */
#if defined(__i386__) || defined(__x86_64__) || defined(__i386) || defined(__amd64__)

/* x86 built-in with hidden visibility */
extern unsigned long long __attribute__((visibility("hidden"), used, artificial))
    __builtin_ia32_rdtsc(void);

/* SSE built-in */
extern __m128 __attribute__((visibility("hidden"), artificial))
    __builtin_ia32_loadups(float const*);

/* MMX built-in (deprecated but still exists) */
extern void __attribute__((visibility("hidden"), used))
    __builtin_ia32_emms(void);

/* Control register access */
extern unsigned int __attribute__((visibility("hidden"), artificial, extern))
    __builtin_ia32_readeflags_u32(void);

#define HAS_TARGET_BUILTINS 1

/* ARM/AArch64 specific built-ins */
#elif defined(__arm__) || defined(__aarch64__) || defined(__ARM_ARCH)

/* ARM CRC32 built-in */
extern unsigned int __attribute__((visibility("hidden"), used, artificial))
    __builtin_arm_crc32b(unsigned int, unsigned char);

/* ARM system register access */
extern unsigned int __attribute__((visibility("hidden"), artificial))
    __builtin_arm_rsr(const char*);

/* AArch64 built-in */
extern unsigned long long __attribute__((visibility("hidden"), used))
    __builtin_aarch64_rdtsc(void);

#define HAS_TARGET_BUILTINS 1

/* PowerPC specific built-ins */
#elif defined(__powerpc__) || defined(__ppc__) || defined(__PPC__)

/* PPC timebase */
extern unsigned long long __attribute__((visibility("hidden"), artificial, used))
    __builtin_ppc_get_timebase(void);

/* PPC mftb */
extern unsigned int __attribute__((visibility("hidden"), artificial))
    __builtin_ppc_mftb(void);

#define HAS_TARGET_BUILTINS 1

/* Generic fallback - use standard GCC built-ins */
#else

/* Use some generic GCC built-ins with hidden visibility */
extern void * __attribute__((visibility("hidden"), used, artificial))
    __builtin_return_address(unsigned int level);

extern int __attribute__((visibility("hidden"), artificial))
    __builtin_popcount(unsigned int x);

extern void __attribute__((visibility("hidden"), used))
    __builtin_unreachable(void);

#define HAS_TARGET_BUILTINS 1

#endif

/* ============================================
   FUNCTION POINTER ARRAY AND OPAQUE OPERATIONS
   ============================================ */

/* Typedef for function pointers */
typedef int (*func_ptr_t)(int);

/* Volatile function pointer array to prevent optimization */
static volatile func_ptr_t volatile_funcs[5];

/* Opaque operation that compiler can't analyze */
static void perform_opaque_operation(int idx, int value) {
    /* Use inline assembly to create opaque dependency */
    asm volatile("" : "+r"(value) : : "memory");
    
    /* Store in global to prevent dead code elimination */
    static volatile int opaque_store;
    opaque_store = value ^ idx;
}

/* ============================================
   MAIN FUNCTION WITH RUNTIME-DEPENDENT LOGIC
   ============================================ */

int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Initialize global seed from argv if available */
    if (argc > 1) {
        global_seed = atoi(argv[1]);
    } else {
        global_seed = get_runtime_value();
    }
    
    /* ============================================
       PHASE 1: Initialize volatile function pointers
       This forces the compiler to process the declarations
       ============================================ */
    
    /* Initialize array with addresses (or NULL placeholders) */
    volatile_funcs[0] = (func_ptr_t)&__hidden_builtin_1;
    volatile_funcs[1] = (func_ptr_t)&__hidden_builtin_2;
    volatile_funcs[2] = (func_ptr_t)&__hidden_builtin_3;
    volatile_funcs[3] = (func_ptr_t)&__hidden_builtin_4;
    volatile_funcs[4] = (func_ptr_t)&__hidden_builtin_5;
    
    /* ============================================
       PHASE 2: Target-specific built-in references
       ============================================ */
    
#if HAS_TARGET_BUILTINS
    
    /* Create volatile pointers to target built-ins */
    volatile void *target_builtin_ptr = NULL;
    
    /* Architecture-specific references */
    #if defined(__i386__) || defined(__x86_64__)
    target_builtin_ptr = (void *)&__builtin_ia32_rdtsc;
    
    /* Call through volatile pointer in unreachable code */
    if (global_seed == 0xDEADBEEF) {  /* Never true at compile time */
        unsigned long long (*volatile fp)(void) = 
            (unsigned long long (*)(void))target_builtin_ptr;
        result = (int)fp();
    }
    
    #elif defined(__arm__) || defined(__aarch64__)
    target_builtin_ptr = (void *)&__builtin_arm_crc32b;
    
    #elif defined(__powerpc__) || defined(__ppc__)
    target_builtin_ptr = (void *)&__builtin_ppc_get_timebase;
    
    #else
    target_builtin_ptr = (void *)&__builtin_return_address;
    
    #endif
    
    /* Opaque comparison that can't be optimized away */
    if (target_builtin_ptr != (void *)((unsigned long)global_seed)) {
        perform_opaque_operation(0, (int)((unsigned long)target_builtin_ptr & 0xFF));
    }
    
#endif  /* HAS_TARGET_BUILTINS */
    
    /* ============================================
       PHASE 3: Loop with runtime-dependent behavior
       ============================================ */
    
    for (int i = 0; i < 5; i++) {
        /* Create runtime-dependent index */
        int idx = (i + global_seed) % 5;
        
        /* Perform opaque operation with the function pointer */
        perform_opaque_operation(idx, (int)((unsigned long)volatile_funcs[idx] & 0xFF));
        
        /* Conditional that depends on runtime value */
        if ((global_seed & (1 << i)) && volatile_funcs[i] != NULL) {
            /* This code is reachable but the condition is runtime-dependent */
            result ^= i * 31;
        }
    }
    
    /* Final opaque operation to use the result */
    volatile int final_result = result;
    asm volatile("" : : "r"(final_result) : "memory");
    
    return final_result & 0x7F;  /* Return non-zero to be safe */
}

/* ============================================
   DUMMY IMPLEMENTATIONS (never actually called)
   These satisfy the external references
   ============================================ */

int __hidden_builtin_1(int x) {
    return x ^ 0x55;
}

int __hidden_builtin_2(int x, int y) {
    return x + y;
}

int __hidden_builtin_3(void) {
    return global_seed;
}

int __hidden_builtin_4(int x) {
    return x * 2;
}

int __hidden_builtin_5(int x) {
    return x / 2;
}
