/* Built-in function visibility test for targhooks.cc coverage */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global volatile variable to prevent optimization */
volatile int global_seed = 0;

/* Opaque function to prevent constant propagation */
int get_input_value(void) {
    return global_seed;
}

/* ============================================
   PROTOTYPES WITH TARGET ATTRIBUTES
   These should trigger built-in processing
   ============================================ */

/* Prototype 1: Full attribute combination */
int __builtin_hidden_test1(int) 
    __attribute__((visibility("hidden"), extern, used, artificial));

/* Prototype 2: Varying attribute order */
int __builtin_hidden_test2(int) 
    __attribute__((extern, visibility("hidden"), used, artificial));

/* Prototype 3: Without 'used' attribute */
int __builtin_hidden_test3(int) 
    __attribute__((visibility("hidden"), extern, artificial));

/* Prototype 4: Only visibility and extern */
int __builtin_hidden_test4(int) 
    __attribute__((visibility("hidden"), extern));

/* Prototype 5: With noinline to ensure separate processing */
int __builtin_hidden_test5(int) 
    __attribute__((visibility("hidden"), extern, used, artificial, noinline));

/* ============================================
   TARGET-SPECIFIC BUILT-IN DECLARATIONS
   These use actual GCC built-ins when available
   ============================================ */

#if defined(__i386__) || defined(__x86_64__)
/* x86/x86-64 specific built-ins */
int __builtin_ia32_addss(int, int) 
    __attribute__((visibility("hidden"), extern, used, artificial));

int __builtin_ia32_mulss(int, int) 
    __attribute__((visibility("hidden"), extern, used, artificial));

int __builtin_ia32_sqrtss(int) 
    __attribute__((visibility("hidden"), extern, used, artificial));

#define HAS_TARGET_BUILTINS 1
typedef int (*x86_builtin_func)(int, int);
x86_builtin_func x86_builtins[] = {
    (x86_builtin_func)__builtin_ia32_addss,
    (x86_builtin_func)__builtin_ia32_mulss,
    NULL
};

#elif defined(__arm__) || defined(__aarch64__)
/* ARM/AArch64 specific built-ins */
int __builtin_arm_qadd(int, int) 
    __attribute__((visibility("hidden"), extern, used, artificial));

int __builtin_arm_qsub(int, int) 
    __attribute__((visibility("hidden"), extern, used, artificial));

int __builtin_arm_qdbl(int) 
    __attribute__((visibility("hidden"), extern, used, artificial));

#define HAS_TARGET_BUILTINS 1
typedef int (*arm_builtin_func)(int, int);
arm_builtin_func arm_builtins[] = {
    (arm_builtin_func)__builtin_arm_qadd,
    (arm_builtin_func)__builtin_arm_qsub,
    NULL
};

#elif defined(__powerpc__) || defined(__ppc__) || defined(__PPC__)
/* PowerPC specific built-ins */
int __builtin_altivec_vaddubm(int, int) 
    __attribute__((visibility("hidden"), extern, used, artificial));

int __builtin_altivec_vmulesb(int, int) 
    __attribute__((visibility("hidden"), extern, used, artificial));

#define HAS_TARGET_BUILTINS 1
typedef int (*ppc_builtin_func)(int, int);
ppc_builtin_func ppc_builtins[] = {
    (ppc_builtin_func)__builtin_altivec_vaddubm,
    (ppc_builtin_func)__builtin_altivec_vmulesb,
    NULL
};

#else
/* Generic fallback - use our prototypes as built-in stand-ins */
#define HAS_TARGET_BUILTINS 0
typedef int (*generic_builtin_func)(int);
generic_builtin_func generic_builtins[] = {
    __builtin_hidden_test1,
    __builtin_hidden_test2,
    __builtin_hidden_test3,
    __builtin_hidden_test4,
    __builtin_hidden_test5,
    NULL
};
#endif

/* ============================================
   FUNCTION POINTER ARRAY WITH VOLATILE STORAGE
   ============================================ */

/* Volatile function pointer to prevent optimization */
#if HAS_TARGET_BUILTINS
#ifdef __i386__
volatile x86_builtin_func volatile_builtin_ptr = 
    (x86_builtin_func)__builtin_ia32_addss;
#elif defined(__arm__)
volatile arm_builtin_func volatile_builtin_ptr = 
    (arm_builtin_func)__builtin_arm_qadd;
#elif defined(__powerpc__)
volatile ppc_builtin_func volatile_builtin_ptr = 
    (ppc_builtin_func)__builtin_altivec_vaddubm;
#endif
#else
volatile generic_builtin_func volatile_builtin_ptr = 
    __builtin_hidden_test1;
#endif

/* ============================================
   MAIN FUNCTION WITH OPAQUE BUILT-IN USAGE
   ============================================ */

int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Use command line input to create non-constant condition */
    if (argc > 1) {
        global_seed = atoi(argv[1]) % 100;
    }
    
    /* Create input-dependent value */
    int input_val = get_input_value();
    
    /* Array of volatile function pointers */
    volatile void* func_ptrs[10];
    int ptr_count = 0;
    
    /* Store addresses of built-ins in volatile array */
#if HAS_TARGET_BUILTINS
#ifdef __i386__
    for (int i = 0; x86_builtins[i] != NULL && i < 5; i++) {
        func_ptrs[ptr_count++] = (volatile void*)x86_builtins[i];
    }
#elif defined(__arm__)
    for (int i = 0; arm_builtins[i] != NULL && i < 5; i++) {
        func_ptrs[ptr_count++] = (volatile void*)arm_builtins[i];
    }
#elif defined(__powerpc__)
    for (int i = 0; ppc_builtins[i] != NULL && i < 5; i++) {
        func_ptrs[ptr_count++] = (volatile void*)ppc_builtins[i];
    }
#endif
#else
    for (int i = 0; generic_builtins[i] != NULL && i < 5; i++) {
        func_ptrs[ptr_count++] = (volatile void*)generic_builtins[i];
    }
#endif
    
    /* Non-optimizable comparison with volatile function pointer */
    if ((void*)volatile_builtin_ptr == func_ptrs[0]) {
        /* This comparison cannot be resolved at compile-time */
        result += 1;
    }
    
    /* Loop with opaque operations on function pointers */
    for (int i = 0; i < ptr_count; i++) {
        /* Create opaque dependency on input */
        if ((input_val & (1 << i)) != 0) {
            /* Cast and potentially call through volatile pointer */
            volatile void* current_ptr = func_ptrs[i];
            
            /* This prevents dead code elimination */
            result += (int)((long)current_ptr & 0x1);
        }
    }
    
    /* Additional opaque use of built-in addresses */
    if (input_val > 50) {
        /* Force compiler to consider all built-in declarations */
        for (int i = 0; i < ptr_count; i++) {
            for (int j = i + 1; j < ptr_count; j++) {
                if (func_ptrs[i] == func_ptrs[j]) {
                    result += 2;
                }
            }
        }
    }
    
    printf("Result: %d (input was: %d)\n", result, input_val);
    
    /* Final opaque use: call through volatile pointer if condition met */
    if (result % 2 == 1) {
#if HAS_TARGET_BUILTINS
        /* For target built-ins that take two arguments */
        int test_val = input_val;
#ifdef __i386__
        if (volatile_builtin_ptr) {
            result = volatile_builtin_ptr(test_val, test_val + 1);
        }
#elif defined(__arm__)
        if (volatile_builtin_ptr) {
            result = volatile_builtin_ptr(test_val, test_val * 2);
        }
#elif defined(__powerpc__)
        if (volatile_builtin_ptr) {
            result = volatile_builtin_ptr(test_val, test_val << 1);
        }
#endif
#else
        /* For generic single-argument prototypes */
        if (volatile_builtin_ptr) {
            result = volatile_builtin_ptr(test_val);
        }
#endif
    }
    
    return result & 0xFF;  /* Return non-zero to indicate execution */
}

/* ============================================
   DUMMY IMPLEMENTATIONS (if not built-in)
   These provide definitions if the compiler doesn't
   recognize them as built-ins
   ============================================ */

int __builtin_hidden_test1(int x) {
    return x + 1;
}

int __builtin_hidden_test2(int x) {
    return x * 2;
}

int __builtin_hidden_test3(int x) {
    return x - 1;
}

int __builtin_hidden_test4(int x) {
    return x / 2;
}

int __builtin_hidden_test5(int x) {
    return x ^ 0xFF;
}
