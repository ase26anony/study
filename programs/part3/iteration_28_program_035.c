/* Program to trigger built-in function declaration with hidden visibility */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global volatile variable to prevent optimization */
volatile int global_seed = 0;

/* ============================================
   DECLARATION OF PROTOTYPES WITH TARGET ATTRIBUTES
   ============================================ */

/* Prototype 1: Full attribute combination */
extern int __attribute__((visibility("hidden"), used, artificial, noinline))
__hidden_builtin_1(int x);

/* Prototype 2: Visibility specified with extern */
extern int __attribute__((visibility("hidden"), extern))
__hidden_builtin_2(int x, int y);

/* Prototype 3: Used + artificial + hidden */
int __attribute__((visibility("hidden"), used, artificial, noinline, noipa))
__hidden_builtin_3(void);

/* Prototype 4: Just hidden visibility */
int __attribute__((visibility("hidden")))
__hidden_builtin_4(int *ptr);

/* ============================================
   TARGET-SPECIFIC BUILT-IN DECLARATIONS
   ============================================ */

/* x86/x86_64 specific built-ins */
#if defined(__i386__) || defined(__x86_64__) || defined(__amd64__)

/* Use actual GCC x86 built-ins */
extern int __attribute__((visibility("hidden"), used, artificial))
__builtin_ia32_rdtsc(void);

extern void __attribute__((visibility("hidden"), used, artificial))
__builtin_ia32_pause(void);

extern unsigned int __attribute__((visibility("hidden"), extern, used))
__builtin_ia32_crc32qi(unsigned int, unsigned char);

/* Declare with target-specific attributes */
#define DECLARE_X86_BUILTIN(name, ret, args) \
    extern ret __attribute__((visibility("hidden"), used, artificial, noinline)) \
    name args

DECLARE_X86_BUILTIN(__builtin_ia32_addcarryx_u32, unsigned char,
                    (unsigned char, unsigned int, unsigned int, unsigned int *));

#endif

/* ARM specific built-ins */
#if defined(__arm__) || defined(__aarch64__) || defined(__ARM_ARCH)

extern unsigned int __attribute__((visibility("hidden"), used, artificial))
__builtin_arm_rbit(unsigned int);

extern int __attribute__((visibility("hidden"), extern, used))
__builtin_arm_clz(unsigned int);

#endif

/* PowerPC specific built-ins */
#if defined(__powerpc__) || defined(__ppc__) || defined(__PPC__)

extern unsigned int __attribute__((visibility("hidden"), used, artificial))
__builtin_ppc_popcntb(unsigned int);

#endif

/* Generic fallback if no target-specific built-ins are available */
#ifndef TARGET_BUILTIN_DEFINED
/* Create dummy built-in-like function */
static inline int __attribute__((visibility("hidden"), always_inline, artificial))
__dummy_builtin(int x) {
    return x ^ 0x55AA55AA;
}
#endif

/* ============================================
   FUNCTION POINTER ARRAY AND OPAQUE OPERATIONS
   ============================================ */

/* Typedef for function pointers */
typedef int (*func_ptr_t)(int);
typedef void (*void_func_ptr_t)(void);

/* Volatile function pointers to prevent optimization */
volatile func_ptr_t volatile_fptr = NULL;
volatile void_func_ptr_t volatile_void_fptr = NULL;

/* Array of function pointers for iteration */
static void* func_ptrs[8] = {0};

/* Opaque operation that compiler can't optimize away */
static int opaque_operation(int x) {
    volatile int result = 0;
    for (int i = 0; i < 4; i++) {
        result ^= (x >> (i * 8)) & 0xFF;
    }
    return result;
}

/* ============================================
   MAIN FUNCTION WITH UNPREDICTABLE CONTROL FLOW
   ============================================ */

int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Use argv to create unpredictable control flow */
    int use_builtin = 0;
    if (argc > 1) {
        use_builtin = opaque_operation(argv[1][0]) & 1;
    }
    
    /* Initialize function pointers based on target */
#if defined(__i386__) || defined(__x86_64__)
    volatile_fptr = (func_ptr_t)__builtin_ia32_rdtsc;
    volatile_void_fptr = (void_func_ptr_t)__builtin_ia32_pause;
    
    /* Store addresses in array */
    func_ptrs[0] = (void*)__builtin_ia32_rdtsc;
    func_ptrs[1] = (void*)__builtin_ia32_pause;
    func_ptrs[2] = (void*)__builtin_ia32_crc32qi;
    
#elif defined(__arm__) || defined(__aarch64__)
    volatile_fptr = (func_ptr_t)__builtin_arm_rbit;
    func_ptrs[0] = (void*)__builtin_arm_rbit;
    func_ptrs[1] = (void*)__builtin_arm_clz;
    
#elif defined(__powerpc__) || defined(__ppc__)
    volatile_fptr = (func_ptr_t)__builtin_ppc_popcntb;
    func_ptrs[0] = (void*)__builtin_ppc_popcntb;
    
#else
    /* Use dummy built-in */
    volatile_fptr = (func_ptr_t)__dummy_builtin;
    func_ptrs[0] = (void*)__dummy_builtin;
#endif
    
    /* Also store our prototype addresses */
    func_ptrs[3] = (void*)__hidden_builtin_1;
    func_ptrs[4] = (void*)__hidden_builtin_2;
    func_ptrs[5] = (void*)__hidden_builtin_3;
    func_ptrs[6] = (void*)__hidden_builtin_4;
    
    /* Unpredictable function call */
    if (use_builtin && volatile_fptr) {
        /* Call through volatile pointer - compiler can't optimize this away */
        result = volatile_fptr(global_seed);
    }
    
    /* Loop through function pointers performing opaque operations */
    for (int i = 0; i < 7; i++) {
        if (func_ptrs[i]) {
            /* Create dependency on function pointer addresses */
            result ^= (long)func_ptrs[i] & 0xFFFF;
            
            /* Compare against known values (non-optimizable) */
            if ((long)func_ptrs[i] == (long)volatile_fptr) {
                result += 0x1000;
            }
        }
    }
    
    /* Force reference to all prototypes to ensure they're processed */
    result += (long)&__hidden_builtin_1 & 1;
    result += (long)&__hidden_builtin_2 & 1;
    result += (long)&__hidden_builtin_3 & 1;
    result += (long)&__hidden_builtin_4 & 1;
    
    /* Call pause built-in if available */
    if (volatile_void_fptr) {
        volatile_void_fptr();
    }
    
    return result & 0xFF;
}

/* ============================================
   DEFINITIONS OF OUR PROTOTYPE FUNCTIONS
   (These simulate what the built-ins would do)
   ============================================ */

int __attribute__((visibility("hidden"), used, artificial, noinline))
__hidden_builtin_1(int x) {
    return x * 2;
}

int __attribute__((visibility("hidden"), extern))
__hidden_builtin_2(int x, int y) {
    return x + y;
}

int __attribute__((visibility("hidden"), used, artificial, noinline, noipa))
__hidden_builtin_3(void) {
    return global_seed;
}

int __attribute__((visibility("hidden")))
__hidden_builtin_4(int *ptr) {
    if (ptr) return *ptr;
    return 0;
}
