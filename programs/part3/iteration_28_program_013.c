/* Built-in function visibility test program */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global volatile variable to prevent optimization */
volatile int global_seed = 0;

/* ============================================
   DECLARATION OF BUILT-IN LIKE FUNCTIONS WITH
   VARIOUS ATTRIBUTE COMBINATIONS
   ============================================ */

/* Prototype 1: Full attribute combination matching target block */
extern int __attribute__((visibility("hidden"), used, artificial))
__hidden_builtin_1(int x) __asm__("__hidden_builtin_1");

/* Prototype 2: Hidden visibility with extern */
extern int __attribute__((visibility("hidden")))
__hidden_builtin_2(int x, int y);

/* Prototype 3: Hidden with used and artificial */
int __attribute__((visibility("hidden"), used, artificial, noinline))
__hidden_builtin_3(void);

/* Prototype 4: Just hidden visibility specified */
int __attribute__((visibility("hidden")))
__hidden_builtin_4(int *ptr);

/* Prototype 5: Combination that should trigger TREE_THIS_VOLATILE */
extern int __attribute__((visibility("hidden"), used, artificial, noreturn))
__hidden_builtin_5(void);

/* ============================================
   TARGET-SPECIFIC BUILT-IN DECLARATIONS
   ============================================ */

#ifdef __i386__
/* x86 specific built-ins */
extern long long __attribute__((visibility("hidden")))
__builtin_ia32_rdtsc(void);

extern void __attribute__((visibility("hidden"), used, artificial))
__builtin_ia32_sfence(void);

extern unsigned int __attribute__((visibility("hidden")))
__builtin_ia32_crc32qi(unsigned int, unsigned char);
#endif

#ifdef __x86_64__
/* x86_64 specific built-ins */
extern void __attribute__((visibility("hidden"), used, artificial))
__builtin_ia32_lfence(void);

extern void __attribute__((visibility("hidden")))
__builtin_ia32_mfence(void);

extern unsigned long long __attribute__((visibility("hidden"), used, artificial))
__builtin_ia32_rdtsc(void);
#endif

#ifdef __arm__
/* ARM specific built-ins */
extern unsigned int __attribute__((visibility("hidden")))
__builtin_arm_rbit(unsigned int);

extern void __attribute__((visibility("hidden"), used, artificial))
__builtin_arm_dmb(unsigned int);

extern unsigned int __attribute__((visibility("hidden")))
__builtin_arm_clz(unsigned int);
#endif

#ifdef __aarch64__
/* AArch64 specific built-ins */
extern unsigned long long __attribute__((visibility("hidden")))
__builtin_aarch64_rdtsc(void);

extern void __attribute__((visibility("hidden"), used, artificial))
__builtin_aarch64_dmb(unsigned int);
#endif

/* ============================================
   FUNCTION POINTER ARRAY FOR OPAQUE OPERATIONS
   ============================================ */

/* Typedef for function pointers */
typedef int (*func_ptr_t)(int);
typedef void (*void_func_ptr_t)(void);

/* Array of function pointers - volatile to prevent optimization */
volatile func_ptr_t func_array[] = {
    (func_ptr_t)__hidden_builtin_1,
    (func_ptr_t)__hidden_builtin_2,
    (func_ptr_t)__hidden_builtin_4,
    NULL
};

volatile void_func_ptr_t void_func_array[] = {
    (void_func_ptr_t)__hidden_builtin_3,
    (void_func_ptr_t)__hidden_builtin_5,
    NULL
};

/* ============================================
   HELPER FUNCTIONS TO CREATE OPAQUE CONDITIONS
   ============================================ */

/* Function that creates input-dependent condition */
static int get_input_value(int argc, char **argv) {
    if (argc > 1) {
        return atoi(argv[1]);
    }
    return 42; /* Default value */
}

/* Function that performs opaque computation */
static int opaque_computation(int x) {
    volatile int result = x;
    for (int i = 0; i < 10; i++) {
        result = (result * 1103515245 + 12345) & 0x7fffffff;
    }
    return result;
}

/* ============================================
   MAIN FUNCTION WITH BUILT-IN USAGE
   ============================================ */

int main(int argc, char **argv) {
    int input = get_input_value(argc, argv);
    int result = 0;
    
    /* Volatile function pointer to prevent optimization */
    volatile func_ptr_t volatile_fp = NULL;
    volatile void_func_ptr_t volatile_void_fp = NULL;
    
    /* Create opaque condition based on input */
    int condition = opaque_computation(input) % 5;
    
    /* Switch based on opaque condition to use different built-ins */
    switch (condition) {
        case 0:
            volatile_fp = (func_ptr_t)__hidden_builtin_1;
            break;
        case 1:
            volatile_fp = (func_ptr_t)__hidden_builtin_2;
            break;
        case 2:
            volatile_void_fp = (void_func_ptr_t)__hidden_builtin_3;
            break;
        case 3:
            volatile_fp = (func_ptr_t)__hidden_builtin_4;
            break;
        case 4:
            volatile_void_fp = (void_func_ptr_t)__hidden_builtin_5;
            break;
    }
    
    /* Use target-specific built-ins based on architecture */
#if defined(__i386__) || defined(__x86_64__)
    /* Use x86 built-ins */
    volatile unsigned long long tsc_value = 0;
    
    #ifdef __i386__
    tsc_value = __builtin_ia32_rdtsc();
    __builtin_ia32_sfence();
    #else
    tsc_value = __builtin_ia32_rdtsc();
    __builtin_ia32_lfence();
    #endif
    
    /* Create opaque use of the value */
    result = (int)(tsc_value % 1000);
    
#elif defined(__arm__) || defined(__aarch64__)
    /* Use ARM built-ins */
    volatile unsigned int arm_value = input;
    
    #ifdef __arm__
    arm_value = __builtin_arm_rbit(arm_value);
    __builtin_arm_dmb(0xF);
    #else
    __builtin_aarch64_dmb(0xF);
    #endif
    
    result = (int)arm_value;
#endif
    
    /* Loop through function pointer array for opaque operations */
    for (int i = 0; func_array[i] != NULL; i++) {
        volatile int temp = func_array[i](i + input);
        result ^= temp; /* Opaque operation */
    }
    
    for (int i = 0; void_func_array[i] != NULL; i++) {
        /* Just calling to ensure compiler processes declaration */
        if ((opaque_computation(i) % 3) == 0) {
            void_func_array[i]();
        }
    }
    
    /* Final opaque use of volatile function pointer */
    if (volatile_fp != NULL && (opaque_computation(result) % 2) == 0) {
        result = volatile_fp(result);
    }
    
    if (volatile_void_fp != NULL && (opaque_computation(result) % 3) == 0) {
        volatile_void_fp();
    }
    
    /* Prevent dead code elimination */
    global_seed = result;
    
    printf("Result: %d\n", result);
    return result % 256;
}

/* ============================================
   DUMMY IMPLEMENTATIONS TO SATISFY LINKER
   (Compiler should replace these with built-ins)
   ============================================ */

int __hidden_builtin_1(int x) {
    return x * 2;
}

int __hidden_builtin_2(int x, int y) {
    return x + y;
}

int __hidden_builtin_3(void) {
    return global_seed;
}

int __hidden_builtin_4(int *ptr) {
    if (ptr) return *ptr;
    return 0;
}

int __hidden_builtin_5(void) {
    /* noreturn function - should never return */
    exit(global_seed);
    return 0;
}
