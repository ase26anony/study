/* Built-in function visibility test program */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global volatile variable to prevent optimization */
volatile int global_seed = 0;

/* ==============================================
 * PHASE 1: Declare prototypes with various attribute combinations
 * These should mimic built-in function declarations
 * ============================================== */

/* Prototype 1: Full attribute set matching target block */
int __hidden_builtin_1(int x) 
    __attribute__((visibility("hidden"), 
                   extern, 
                   used, 
                   artificial, 
                   noinline, 
                   noreturn, 
                   nothrow));

/* Prototype 2: Partial attribute set */
int __hidden_builtin_2(int x, int y) 
    __attribute__((visibility("hidden"), 
                   used, 
                   noinline));

/* Prototype 3: Extern without visibility specified */
int __hidden_builtin_3(void) 
    __attribute__((extern, 
                   used, 
                   artificial));

/* Prototype 4: Just hidden visibility */
int __hidden_builtin_4(int *ptr) 
    __attribute__((visibility("hidden")));

/* ==============================================
 * PHASE 2: Target-specific built-in declarations
 * Using actual GCC built-ins for different architectures
 * ============================================== */

/* x86/x86_64 specific built-ins */
#if defined(__i386__) || defined(__x86_64__) || defined(__i686__)

/* Use Intel intrinsics that map to built-ins */
int __builtin_ia32_rdtsc(void) 
    __attribute__((visibility("hidden"), 
                   extern, 
                   used, 
                   artificial));

unsigned int __builtin_ia32_crc32qi(unsigned int, unsigned char)
    __attribute__((visibility("hidden"),
                   used,
                   artificial));

void __builtin_ia32_pause(void)
    __attribute__((visibility("hidden"),
                   extern,
                   used));

/* x86 SIMD built-in */
int __builtin_ia32_addss(int, int)
    __attribute__((visibility("hidden"),
                   extern,
                   used,
                   artificial));

#define HAS_TARGET_BUILTINS 1
#define TARGET_BUILTIN_FUNC __builtin_ia32_rdtsc

/* ARM specific built-ins */
#elif defined(__arm__) || defined(__aarch64__) || defined(__ARM_ARCH)

/* ARM CRC32 built-in */
unsigned int __builtin_arm_crc32b(unsigned int, unsigned int)
    __attribute__((visibility("hidden"),
                   extern,
                   used,
                   artificial));

/* ARM data barrier */
void __builtin_arm_dmb(unsigned int)
    __attribute__((visibility("hidden"),
                   used));

unsigned int __builtin_arm_rbit(unsigned int)
    __attribute__((visibility("hidden"),
                   extern,
                   used,
                   artificial));

#define HAS_TARGET_BUILTINS 1
#define TARGET_BUILTIN_FUNC __builtin_arm_crc32b

/* PowerPC specific built-ins */
#elif defined(__powerpc__) || defined(__ppc__) || defined(__PPC__)

int __builtin_ppc_mftb(void)
    __attribute__((visibility("hidden"),
                   extern,
                   used,
                   artificial));

unsigned long long __builtin_ppc_get_timebase(void)
    __attribute__((visibility("hidden"),
                   used,
                   artificial));

#define HAS_TARGET_BUILTINS 1
#define TARGET_BUILTIN_FUNC __builtin_ppc_mftb

/* Generic fallback - use GCC built-ins */
#else

/* Use some generic GCC built-ins */
int __builtin_ffs(int)
    __attribute__((visibility("hidden"),
                   extern,
                   used,
                   artificial));

int __builtin_clz(unsigned int)
    __attribute__((visibility("hidden"),
                   used,
                   artificial));

void __builtin_unreachable(void)
    __attribute__((visibility("hidden"),
                   extern,
                   used,
                   artificial));

#define HAS_TARGET_BUILTINS 1
#define TARGET_BUILTIN_FUNC __builtin_ffs

#endif

/* ==============================================
 * PHASE 3: Function pointer declarations and usage
 * ============================================== */

/* Typedef for function pointers */
typedef int (*func_ptr_t)(int);

/* Array of function pointers - some will be built-ins */
static func_ptr_t func_table[] = {
    (func_ptr_t)__hidden_builtin_1,
    (func_ptr_t)__hidden_builtin_2,
    (func_ptr_t)__hidden_builtin_4,
#ifdef HAS_TARGET_BUILTINS
    (func_ptr_t)TARGET_BUILTIN_FUNC,
#endif
    NULL
};

/* Volatile function pointer to prevent optimization */
volatile func_ptr_t volatile_fp = NULL;

/* Opaque function to confuse the optimizer */
static int opaque_transform(int x) {
    return (x * 1103515245 + 12345) & 0x7fffffff;
}

/* ==============================================
 * PHASE 4: Main function with non-optimizable logic
 * ============================================== */

int main(int argc, char *argv[]) {
    int result = 0;
    int i;
    
    /* Use argv to create input-dependent behavior */
    int input_seed = 0;
    if (argc > 1) {
        input_seed = atoi(argv[1]);
    }
    
    /* Initialize volatile global */
    global_seed = input_seed;
    
    /* Assign built-in function address to volatile pointer */
#ifdef HAS_TARGET_BUILTINS
    volatile_fp = (func_ptr_t)TARGET_BUILTIN_FUNC;
#else
    volatile_fp = (func_ptr_t)__hidden_builtin_1;
#endif
    
    /* Non-optimizable conditional based on input */
    if (input_seed > 1000) {
        /* This branch forces compiler to consider the function pointer */
        if (volatile_fp != NULL) {
            /* Call through volatile pointer - compiler can't optimize this away */
            result = volatile_fp(input_seed);
        }
    } else {
        /* Alternative path that also references built-ins */
        result = opaque_transform(input_seed);
        
        /* Compare function addresses in a way that can't be resolved at compile time */
        for (i = 0; func_table[i] != NULL; i++) {
            if ((unsigned long)func_table[i] == (unsigned long)opaque_transform) {
                result ^= 0xDEADBEEF;
            }
        }
    }
    
    /* Loop that processes function pointers */
    for (i = 0; func_table[i] != NULL; i++) {
        /* Opaque operation on function addresses */
        result ^= (unsigned long)func_table[i] & 0xFF;
        
        /* Conditional that might call the function */
        if ((global_seed + i) % 7 == 0) {
            /* This creates a potential call site that the compiler must prepare for */
            volatile int temp = (unsigned long)func_table[i];
            result += temp;
        }
    }
    
    /* Additional reference to ensure built-in is marked used */
    asm volatile("" : : "r"(__hidden_builtin_1), "r"(__hidden_builtin_2));
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return result != 0 ? 0 : 1;
}

/* ==============================================
 * PHASE 5: Function definitions (some may remain undefined)
 * ============================================== */

/* Define one of the functions to avoid linker errors */
int __hidden_builtin_2(int x, int y) {
    return x + y + global_seed;
}

int __hidden_builtin_4(int *ptr) {
    if (ptr) {
        return *ptr + 1;
    }
    return 0;
}
