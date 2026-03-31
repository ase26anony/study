/* Built-in function visibility test to trigger target hook logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global volatile variable to prevent optimization */
volatile int global_seed = 0;

/* Opaque function to prevent constant propagation */
static int get_input_value(void) {
    return global_seed;
}

/* ============================================
   PROTOTYPES WITH VISIBILITY ATTRIBUTES
   ============================================ */

/* Prototype 1: Full attribute combination matching target block */
extern int __hidden_builtin_1(int) 
    __attribute__((visibility("hidden"), used, artificial, noinline, noreturn));

/* Prototype 2: Different attribute ordering */
int __hidden_builtin_2(float) 
    __attribute__((extern, visibility("hidden"), used, artificial));

/* Prototype 3: Without 'extern' keyword but with attribute */
int __hidden_builtin_3(void) 
    __attribute__((visibility("hidden"), used, artificial));

/* Prototype 4: With volatile pointer parameter */
void __hidden_builtin_4(volatile int*) 
    __attribute__((visibility("hidden"), extern, used, artificial));

/* ============================================
   TARGET-SPECIFIC BUILT-IN DECLARATIONS
   ============================================ */

/* x86/x86-64 specific built-ins */
#if defined(__i386__) || defined(__x86_64__) || defined(__amd64__)

/* Use actual GCC x86 built-ins that go through builtin_function_ext_scope */
extern int __builtin_ia32_rdtsc(void) 
    __attribute__((visibility("hidden"), used, artificial));

extern void __builtin_ia32_pause(void) 
    __attribute__((visibility("hidden"), extern, used, artificial));

extern unsigned char __builtin_ia32_bsrsi(unsigned int) 
    __attribute__((visibility("hidden"), used, artificial));

extern long long __builtin_ia32_rdpmc(int) 
    __attribute__((visibility("hidden"), extern, used, artificial));

#define HAS_TARGET_BUILTINS 1

/* ARM specific built-ins */
#elif defined(__arm__) || defined(__aarch64__) || defined(__ARM_ARCH)

extern unsigned int __builtin_arm_rbit(unsigned int)
    __attribute__((visibility("hidden"), used, artificial));

extern void __builtin_arm_dmb(unsigned int)
    __attribute__((visibility("hidden"), extern, used, artificial));

extern unsigned int __builtin_arm_clz(unsigned int)
    __attribute__((visibility("hidden"), used, artificial));

#define HAS_TARGET_BUILTINS 1

/* PowerPC specific built-ins */
#elif defined(__powerpc__) || defined(__ppc__) || defined(__PPC__)

extern unsigned int __builtin_ppc_mftb(void)
    __attribute__((visibility("hidden"), used, artificial));

extern void __builtin_ppc_sync(void)
    __attribute__((visibility("hidden"), extern, used, artificial));

#define HAS_TARGET_BUILTINS 1

/* Generic fallback - use GCC generic built-ins */
#else

/* Use GCC generic built-ins with hidden visibility */
extern void *__builtin_alloca(size_t)
    __attribute__((visibility("hidden"), used, artificial));

extern int __builtin_clz(unsigned int)
    __attribute__((visibility("hidden"), extern, used, artificial));

extern void __builtin_prefetch(const void*, ...)
    __attribute__((visibility("hidden"), used, artificial));

#define HAS_TARGET_BUILTINS 1

#endif

/* ============================================
   FUNCTION POINTER ARRAY WITH VOLATILE STORAGE
   ============================================ */

/* Typedef for function pointers */
typedef int (*func_ptr_int_t)(int);
typedef void (*func_ptr_void_t)(void);

/* Volatile function pointers to prevent optimization */
volatile func_ptr_int_t volatile_fp1 = 0;
volatile func_ptr_int_t volatile_fp2 = 0;
volatile func_ptr_void_t volatile_fp3 = 0;

/* Array of function pointers for iteration */
static void* func_array[] = {
    (void*)__hidden_builtin_1,
    (void*)__hidden_builtin_2,
    (void*)__hidden_builtin_3,
    (void*)__hidden_builtin_4,
#if HAS_TARGET_BUILTINS
#ifdef __i386__
    (void*)__builtin_ia32_rdtsc,
    (void*)__builtin_ia32_pause,
#elif defined(__arm__)
    (void*)__builtin_arm_rbit,
    (void*)__builtin_arm_dmb,
#elif defined(__powerpc__)
    (void*)__builtin_ppc_mftb,
    (void*)__builtin_ppc_sync,
#else
    (void*)__builtin_alloca,
    (void*)__builtin_clz,
#endif
#endif
    NULL
};

/* ============================================
   MAIN FUNCTION WITH NON-OPTIMIZABLE LOGIC
   ============================================ */

int main(int argc, char *argv[]) {
    /* Use argv to create input-dependent behavior */
    int use_builtin = 0;
    if (argc > 1) {
        use_builtin = (argv[1][0] != '\0');
        global_seed = argv[1][0]; /* Seed from input */
    }
    
    /* Initialize volatile function pointers with built-in addresses */
    volatile_fp1 = (func_ptr_int_t)__hidden_builtin_1;
    
#if HAS_TARGET_BUILTINS
#ifdef __i386__
    volatile_fp2 = (func_ptr_int_t)__builtin_ia32_rdtsc;
    volatile_fp3 = (func_ptr_void_t)__builtin_ia32_pause;
#elif defined(__arm__)
    volatile_fp2 = (func_ptr_int_t)__builtin_arm_rbit;
    volatile_fp3 = (func_ptr_void_t)__builtin_arm_dmb;
#elif defined(__powerpc__)
    volatile_fp2 = (func_ptr_int_t)__builtin_ppc_mftb;
    volatile_fp3 = (func_ptr_void_t)__builtin_ppc_sync;
#else
    volatile_fp2 = (func_ptr_int_t)__builtin_clz;
    volatile_fp3 = (func_ptr_void_t)__builtin_prefetch;
#endif
#endif
    
    /* Non-optimizable conditional using volatile variables */
    int result = 0;
    int input_val = get_input_value();
    
    if (use_builtin && input_val != 0) {
        /* This comparison cannot be resolved at compile time */
        if ((void*)volatile_fp1 != (void*)volatile_fp2) {
            /* Force compiler to process both function declarations */
            result = 1;
        }
    }
    
    /* Loop through function pointer array - opaque to optimizer */
    for (int i = 0; func_array[i] != NULL; i++) {
        /* Opaque operation that references each function */
        if ((long)func_array[i] & 1) {
            result ^= 1; /* Unpredictable at compile time */
        }
    }
    
    /* Create a conditional that might call through volatile pointer */
    if (result && input_val > 64) {
        /* The compiler must keep the built-in reference */
        func_ptr_int_t local_fp = (func_ptr_int_t)volatile_fp1;
        /* Note: We don't actually call it to avoid runtime issues */
    }
    
    return result;
}

/* ============================================
   DUMMY DEFINITIONS TO SATISFY LINKER
   (These won't be used if built-ins are recognized)
   ============================================ */

int __hidden_builtin_1(int x) {
    return x + 1;
}

int __hidden_builtin_2(float f) {
    return (int)f;
}

int __hidden_builtin_3(void) {
    return 42;
}

void __hidden_builtin_4(volatile int* p) {
    if (p) *p = 0;
}
