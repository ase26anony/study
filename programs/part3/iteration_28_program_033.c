/* Built-in function declaration test for targhooks.cc coverage */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global volatile variable to prevent optimization */
volatile int global_seed = 0;

/* ============================================
   PROTOTYPE DECLARATIONS WITH TARGET ATTRIBUTES
   ============================================ */

/* Declaration 1: Full attribute combination */
extern int __attribute__((visibility("hidden"), used, artificial, noinline))
__hidden_builtin_1(int x) __asm__("__hidden_builtin_1");

/* Declaration 2: Different attribute order */
extern int __attribute__((used, artificial, visibility("hidden")))
__hidden_builtin_2(int x, int y) __asm__("__hidden_builtin_2");

/* Declaration 3: With no_return attribute */
extern void __attribute__((visibility("hidden"), noreturn, used, artificial))
__hidden_builtin_noreturn(void) __asm__("__hidden_builtin_noreturn");

/* Declaration 4: With const attribute */
extern int __attribute__((visibility("hidden"), const, used, artificial))
__hidden_builtin_const(void) __asm__("__hidden_builtin_const");

/* Declaration 5: Pure function */
extern int __attribute__((visibility("hidden"), pure, used, artificial))
__hidden_builtin_pure(int x) __asm__("__hidden_builtin_pure");

/* ============================================
   TARGET-SPECIFIC BUILT-IN DECLARATIONS
   ============================================ */

/* x86/x86-64 specific built-ins */
#if defined(__i386__) || defined(__x86_64__) || defined(__amd64__)

/* Use various x86 built-ins that might go through builtin_function_ext_scope */
extern unsigned long long __attribute__((visibility("hidden"), used, artificial))
__builtin_ia32_rdtsc(void);

extern void __attribute__((visibility("hidden"), used, artificial))
__builtin_ia32_mfence(void);

extern int __attribute__((visibility("hidden"), used, artificial))
__builtin_ia32_addcarryx_u32(unsigned char __cf, unsigned int __x,
                             unsigned int __y, unsigned int *__p);

extern unsigned int __attribute__((visibility("hidden"), used, artificial))
__builtin_ia32_crc32qi(unsigned int __C, unsigned char __D);

/* x86 SIMD built-ins */
extern int __attribute__((visibility("hidden"), used, artificial))
__builtin_ia32_paddb128(__v16qi __A, __v16qi __B);

#endif /* x86/x86-64 */

/* ARM specific built-ins */
#if defined(__arm__) || defined(__aarch64__) || defined(__ARM_ARCH)

extern unsigned int __attribute__((visibility("hidden"), used, artificial))
__builtin_arm_rbit(unsigned int);

extern void __attribute__((visibility("hidden"), used, artificial))
__builtin_arm_dmb(unsigned int);

extern unsigned int __attribute__((visibility("hidden"), used, artificial))
__builtin_arm_clz(unsigned int);

#endif /* ARM */

/* PowerPC specific built-ins */
#if defined(__powerpc__) || defined(__ppc__) || defined(__PPC__)

extern unsigned int __attribute__((visibility("hidden"), used, artificial))
__builtin_ppc_mftb(void);

extern int __attribute__((visibility("hidden"), used, artificial))
__builtin_ppc_popcntb(unsigned int);

#endif /* PowerPC */

/* ============================================
   FUNCTION POINTER ARRAY AND OPAQUE OPERATIONS
   ============================================ */

/* Volatile function pointer to prevent optimization */
typedef int (*volatile func_ptr_t)(int);

/* Array of function pointers for iteration */
static func_ptr_t volatile func_ptrs[5];

/* Opaque operation that compiler can't optimize away */
static int opaque_operation(int x) {
    volatile int result = 0;
    for (volatile int i = 0; i < 10; i++) {
        result += x + i;
    }
    return result;
}

/* ============================================
   MAIN FUNCTION WITH COMPLEX CONTROL FLOW
   ============================================ */

int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Use argv to create input-dependent control flow */
    int use_builtin = 0;
    if (argc > 1) {
        use_builtin = (argv[1][0] != '\0');
        global_seed = argv[1][0];  /* Prevent optimization */
    }
    
    /* Initialize function pointer array with various targets */
    func_ptrs[0] = (func_ptr_t)__hidden_builtin_1;
    func_ptrs[1] = (func_ptr_t)__hidden_builtin_2;
    func_ptrs[2] = (func_ptr_t)__hidden_builtin_pure;
    
    /* Target-specific built-in assignments */
#if defined(__i386__) || defined(__x86_64__) || defined(__amd64__)
    /* x86 built-in function pointers */
    func_ptrs[3] = (func_ptr_t)__builtin_ia32_crc32qi;
    /* Take address of volatile built-in */
    volatile unsigned long long (*volatile rdtsc_ptr)(void) = __builtin_ia32_rdtsc;
    if (rdtsc_ptr) {
        result += 1;  /* Side effect */
    }
#endif
    
#if defined(__arm__) || defined(__aarch64__) || defined(__ARM_ARCH__)
    /* ARM built-in function pointers */
    func_ptrs[4] = (func_ptr_t)__builtin_arm_clz;
#endif
    
    /* Complex loop with opaque operations */
    for (volatile int i = 0; i < 5; i++) {
        if (func_ptrs[i] != NULL) {
            /* Create non-optimizable condition */
            if ((global_seed + i) % 3 == 0) {
                /* Call through volatile function pointer */
                int temp = func_ptrs[i](i + global_seed);
                result += opaque_operation(temp);
            } else if ((global_seed + i) % 3 == 1) {
                /* Compare function pointers */
                if (func_ptrs[i] != (func_ptr_t)__hidden_builtin_1) {
                    result += i * 2;
                }
            } else {
                /* More opaque operations */
                result += opaque_operation(i);
            }
        }
    }
    
    /* Additional target-specific operations */
#if defined(__i386__) || defined(__x86_64__) || defined(__amd64__)
    if (use_builtin) {
        /* Direct use of x86 built-ins with side effects */
        __builtin_ia32_mfence();
        unsigned int crc = 0;
        __builtin_ia32_addcarryx_u32(0, 100, 200, &crc);
        result += crc;
    }
#endif
    
#if defined(__arm__) || defined(__aarch64__) || defined(__ARM_ARCH__)
    if (use_builtin) {
        unsigned int rbit_val = __builtin_arm_rbit(0x12345678);
        result += rbit_val & 0xFF;
    }
#endif
    
    /* Final opaque computation */
    result = opaque_operation(result);
    
    printf("Result: %d (seed: %d)\n", result, global_seed);
    return result & 0xFF;  /* Return non-zero to ensure execution */
}

/* ============================================
   WEAK ALIASES AND ADDITIONAL DECLARATIONS
   ============================================ */

/* Weak aliases to potentially trigger different code paths */
extern int __attribute__((weak, visibility("hidden"), used, artificial))
__weak_hidden_builtin(int) __asm__("__weak_hidden_builtin");

/* Another declaration with assembly name */
extern int __attribute__((visibility("hidden"), used, artificial))
__asm_builtin(void) __asm__("___asm_builtin");

/* Declaration with deprecated attribute */
extern int __attribute__((visibility("hidden"), used, artificial, deprecated))
__deprecated_hidden_builtin(int) __asm__("__deprecated_hidden_builtin");
