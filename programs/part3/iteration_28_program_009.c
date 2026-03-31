/* Built-in function visibility test for targhooks.cc coverage */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global volatile variable to prevent optimization */
volatile int global_seed = 0;

/* Opaque function to prevent constant propagation */
int get_input_value(int argc, char **argv) {
    if (argc > 1) return atoi(argv[1]);
    return __LINE__;
}

/* ============================================================
   PROTOTYPES WITH VISIBILITY ATTRIBUTES
   ============================================================ */

/* Prototype 1: Full attribute combination targeting the uncovered block */
int __hidden_builtin_1(int x) 
    __attribute__((visibility("hidden"), 
                   extern, 
                   used, 
                   artificial, 
                   noinline, 
                   noreturn));

/* Prototype 2: Different attribute ordering */
int __hidden_builtin_2(int x, int y) 
    __attribute__((extern, 
                   used, 
                   visibility("hidden"), 
                   artificial));

/* Prototype 3: With target-specific attributes if available */
int __hidden_builtin_3(void) 
    __attribute__((visibility("hidden"), 
                   used, 
                   artificial, 
                   const));

/* Prototype 4: Simulating built-in declaration pattern */
int __hidden_builtin_4(float f) 
    __attribute__((visibility("hidden"), 
                   extern, 
                   artificial, 
                   leaf));

/* ============================================================
   TARGET-SPECIFIC BUILT-IN DECLARATIONS
   ============================================================ */

#if defined(__i386__) || defined(__x86_64__)
/* x86/x86-64 specific built-ins */
int __builtin_ia32_rdtsc(void) 
    __attribute__((visibility("hidden"), 
                   extern, 
                   used, 
                   artificial));

void __builtin_ia32_sfence(void) 
    __attribute__((visibility("hidden"), 
                   artificial, 
                   used));

int __builtin_ia32_addcarryx_u32(unsigned char __cf, 
                                 unsigned int __x, 
                                 unsigned int __y, 
                                 unsigned int *__p) 
    __attribute__((visibility("hidden"), 
                   extern, 
                   used, 
                   artificial));

/* MMX/SSE built-ins */
__m128 __builtin_ia32_addps(__m128 __a, __m128 __b) 
    __attribute__((visibility("hidden"), 
                   extern, 
                   used, 
                   artificial));

#elif defined(__arm__) || defined(__aarch64__)
/* ARM/AArch64 specific built-ins */
unsigned int __builtin_arm_rbit(unsigned int __x) 
    __attribute__((visibility("hidden"), 
                   extern, 
                   used, 
                   artificial));

unsigned int __builtin_arm_clz(unsigned int __x) 
    __attribute__((visibility("hidden"), 
                   artificial, 
                   used));

void __builtin_arm_dmb(unsigned int __x) 
    __attribute__((visibility("hidden"), 
                   extern, 
                   used, 
                   artificial));

#elif defined(__powerpc__) || defined(__ppc__) || defined(__PPC__)
/* PowerPC specific built-ins */
unsigned int __builtin_ppc_mftb(void) 
    __attribute__((visibility("hidden"), 
                   extern, 
                   used, 
                   artificial));

void __builtin_ppc_sync(void) 
    __attribute__((visibility("hidden"), 
                   artificial, 
                   used));

#else
/* Generic fallback built-ins */
int __builtin_abs(int __x) 
    __attribute__((visibility("hidden"), 
                   extern, 
                   used, 
                   artificial));

int __builtin_clz(unsigned int __x) 
    __attribute__((visibility("hidden"), 
                   artificial, 
                   used));
#endif

/* ============================================================
   FUNCTION POINTER ARRAY WITH VOLATILE QUALIFIERS
   ============================================================ */

/* Volatile function pointer array to prevent optimization */
typedef int (*volatile func_ptr_volatile_t)(int);
typedef void (*volatile func_ptr_void_t)(void);

/* Array of volatile function pointers */
static func_ptr_volatile_t volatile func_ptrs[8];
static func_ptr_void_t volatile void_func_ptrs[4];

/* ============================================================
   MAIN FUNCTION WITH NON-OPTIMIZABLE LOGIC
   ============================================================ */

int main(int argc, char **argv) {
    int input = get_input_value(argc, argv);
    int result = 0;
    
    /* Initialize volatile function pointers with built-in addresses */
    
#if defined(__i386__) || defined(__x86_64__)
    /* x86 built-ins */
    func_ptrs[0] = (func_ptr_volatile_t)__builtin_ia32_rdtsc;
    void_func_ptrs[0] = (func_ptr_void_t)__builtin_ia32_sfence;
    
#elif defined(__arm__) || defined(__aarch64__)
    /* ARM built-ins */
    func_ptrs[0] = (func_ptr_volatile_t)__builtin_arm_rbit;
    func_ptrs[1] = (func_ptr_volatile_t)__builtin_arm_clz;
    
#elif defined(__powerpc__) || defined(__ppc__) || defined(__PPC__)
    /* PowerPC built-ins */
    func_ptrs[0] = (func_ptr_volatile_t)__builtin_ppc_mftb;
    void_func_ptrs[0] = (func_ptr_void_t)__builtin_ppc_sync;
    
#else
    /* Generic built-ins */
    func_ptrs[0] = (func_ptr_volatile_t)__builtin_abs;
    func_ptrs[1] = (func_ptr_volatile_t)__builtin_clz;
#endif
    
    /* Also assign our hidden prototypes */
    func_ptrs[2] = (func_ptr_volatile_t)__hidden_builtin_1;
    func_ptrs[3] = (func_ptr_volatile_t)__hidden_builtin_2;
    func_ptrs[4] = (func_ptr_volatile_t)__hidden_builtin_3;
    func_ptrs[5] = (func_ptr_volatile_t)__hidden_builtin_4;
    
    /* Non-optimizable loop using volatile pointers */
    for (int i = 0; i < 6; i++) {
        if (func_ptrs[i] != NULL) {
            /* Create non-constant condition */
            if ((input + i) % 3 == 0) {
                /* Call through volatile pointer */
                result += func_ptrs[i](input + i);
            } else if ((input + i) % 3 == 1) {
                /* Compare addresses - can't be optimized away */
                if (func_ptrs[i] == func_ptrs[(i + 1) % 6]) {
                    result += 1;
                }
            }
        }
    }
    
    /* Use void function pointers */
    for (int i = 0; i < 4; i++) {
        if (void_func_ptrs[i] != NULL && (input + i) % 5 == 0) {
            void_func_ptrs[i]();
            result += 2;
        }
    }
    
    /* Additional non-optimizable reference to built-ins */
    volatile func_ptr_volatile_t volatile_ptr = NULL;
    
#if defined(__i386__) || defined(__x86_64__)
    volatile_ptr = (func_ptr_volatile_t)__builtin_ia32_addcarryx_u32;
#elif defined(__arm__) || defined(__aarch64__)
    volatile_ptr = (func_ptr_volatile_t)__builtin_arm_dmb;
#endif
    
    if (volatile_ptr != NULL && input % 7 == 0) {
        unsigned int dummy = 0;
        result += volatile_ptr(0, input, input * 2, &dummy);
    }
    
    /* Prevent dead code elimination */
    if (result == 0) {
        /* Reference all prototypes to ensure they're processed */
        asm volatile("" : : "r"(__hidden_builtin_1), "r"(__hidden_builtin_2),
                         "r"(__hidden_builtin_3), "r"(__hidden_builtin_4));
    }
    
    printf("Result: %d (input was %d)\n", result, input);
    return result != 0 ? 0 : 1;
}

/* ============================================================
   DUMMY IMPLEMENTATIONS (NEVER CALLED, BUT NEEDED FOR LINKING)
   ============================================================ */

int __hidden_builtin_1(int x) {
    /* Should never be called - just for declaration processing */
    return x + 1;
}

int __hidden_builtin_2(int x, int y) {
    return x + y;
}

int __hidden_builtin_3(void) {
    return global_seed;
}

int __hidden_builtin_4(float f) {
    return (int)f;
}
