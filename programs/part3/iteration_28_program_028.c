/* Built-in function visibility test program */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global volatile variable to prevent optimization */
volatile int global_seed = 0;

/* Function to create runtime-dependent condition */
static int get_runtime_value(void) {
    return global_seed & 1;
}

/* ============================================
   DECLARE PROTOTYPES WITH TARGET ATTRIBUTES
   ============================================ */

/* Prototype 1: Full attribute combination matching uncovered lines */
extern int __attribute__((visibility("hidden"), used, artificial, noinline, noreturn))
__hidden_builtin_proto1(int x);

/* Prototype 2: Similar but with extern explicitly specified */
extern int __attribute__((visibility("hidden"), used, artificial, noinline))
__hidden_builtin_proto2(int x, int y);

/* Prototype 3: With volatile and nothrow attributes */
extern int __attribute__((visibility("hidden"), used, artificial, noinline, nothrow))
__hidden_builtin_proto3(void);

/* Prototype 4: Minimal attributes but hidden visibility */
extern int __attribute__((visibility("hidden")))
__hidden_builtin_proto4(void);

/* ============================================
   TARGET-SPECIFIC BUILT-IN DECLARATIONS
   ============================================ */

/* x86/x86_64 specific built-ins */
#if defined(__i386__) || defined(__x86_64__) || defined(__amd64__)

/* Declare actual x86 built-ins with hidden visibility */
extern long long __attribute__((visibility("hidden"), used, artificial))
__builtin_ia32_rdtsc(void);

extern void __attribute__((visibility("hidden"), used, artificial))
__builtin_ia32_clflush(const void *);

extern int __attribute__((visibility("hidden"), used, artificial))
__builtin_ia32_addcarryx_u32(unsigned char, unsigned int, unsigned int, unsigned int *);

extern unsigned int __attribute__((visibility("hidden"), used, artificial))
__builtin_ia32_crc32qi(unsigned int, unsigned char);

/* x86 SIMD built-ins */
extern __attribute__((visibility("hidden"), used, artificial)) 
int __builtin_ia32_comieq(__m128, __m128);

#endif

/* ARM specific built-ins */
#if defined(__arm__) || defined(__aarch64__) || defined(__ARM_ARCH)

extern unsigned int __attribute__((visibility("hidden"), used, artificial))
__builtin_arm_rbit(unsigned int);

extern void __attribute__((visibility("hidden"), used, artificial))
__builtin_arm_dmb(unsigned int);

extern unsigned int __attribute__((visibility("hidden"), used, artificial))
__builtin_arm_clz(unsigned int);

#endif

/* PowerPC specific built-ins */
#if defined(__powerpc__) || defined(__ppc__) || defined(__PPC__)

extern unsigned int __attribute__((visibility("hidden"), used, artificial))
__builtin_ppc_popcntb(unsigned int);

extern double __attribute__((visibility("hidden"), used, artificial))
__builtin_ppc_fabs(double);

#endif

/* ============================================
   FUNCTION POINTER ARRAY WITH VOLATILE STORAGE
   ============================================ */

/* Typedef for function pointers */
typedef int (*func_ptr_t)(void);
typedef void (*void_func_ptr_t)(void);

/* Volatile function pointer array to prevent optimization */
static volatile func_ptr_t volatile_funcs[8];
static volatile void_func_ptr_t volatile_void_funcs[8];

/* ============================================
   MAIN FUNCTION WITH NON-OPTIMIZABLE LOGIC
   ============================================ */

int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Initialize global_seed from argv to create runtime dependency */
    if (argc > 1) {
        global_seed = atoi(argv[1]);
    } else {
        global_seed = 12345;
    }
    
    /* Store addresses in volatile function pointers */
    volatile_funcs[0] = (func_ptr_t)__hidden_builtin_proto1;
    volatile_funcs[1] = (func_ptr_t)__hidden_builtin_proto2;
    volatile_funcs[2] = (func_ptr_t)__hidden_builtin_proto3;
    volatile_funcs[3] = (func_ptr_t)__hidden_builtin_proto4;
    
    /* ============================================
       TARGET-SPECIFIC BUILT-IN REFERENCES
       ============================================ */
    
#if defined(__i386__) || defined(__x86_64__) || defined(__amd64__)
    /* Reference x86 built-ins */
    volatile_funcs[4] = (func_ptr_t)__builtin_ia32_rdtsc;
    volatile_void_funcs[0] = (void_func_ptr_t)__builtin_ia32_clflush;
    
    /* Create a non-optimizable reference to the built-in */
    if (get_runtime_value()) {
        unsigned int carry_out;
        unsigned int sum = __builtin_ia32_addcarryx_u32(0, 100, 200, &carry_out);
        result += sum;
    }
    
    /* Use volatile pointer to call built-in */
    if (volatile_funcs[4]) {
        long long tsc = ((long long (*)(void))volatile_funcs[4])();
        result += (int)(tsc & 0xFF);
    }
#endif
    
#if defined(__arm__) || defined(__aarch64__) || defined(__ARM_ARCH)
    /* Reference ARM built-ins */
    volatile_funcs[5] = (func_ptr_t)__builtin_arm_rbit;
    volatile_funcs[6] = (func_ptr_t)__builtin_arm_clz;
    
    if (get_runtime_value()) {
        unsigned int rbit_result = __builtin_arm_rbit(0x12345678);
        result += rbit_result & 1;
    }
#endif
    
#if defined(__powerpc__) || defined(__ppc__) || defined(__PPC__)
    /* Reference PowerPC built-ins */
    volatile_funcs[7] = (func_ptr_t)__builtin_ppc_popcntb;
    
    if (get_runtime_value()) {
        unsigned int popcnt = __builtin_ppc_popcntb(0xFF);
        result += popcnt;
    }
#endif
    
    /* ============================================
       OPAUSE LOOP TO PROCESS FUNCTION POINTERS
       ============================================ */
    
    /* Loop through volatile function pointers */
    for (int i = 0; i < 8; i++) {
        if (volatile_funcs[i]) {
            /* Create opaque operation that compiler can't optimize away */
            if ((global_seed >> i) & 1) {
                /* This comparison is runtime-dependent */
                if ((unsigned long)volatile_funcs[i] > 0x1000) {
                    result += i;
                }
            }
        }
    }
    
    /* Final runtime-dependent output */
    printf("Result: %d (seed: %d)\n", result, global_seed);
    
    return result & 1;
}

/* ============================================
   DUMMY IMPLEMENTATIONS TO SATISFY LINKER
   ============================================ */

/* These are never actually called, but satisfy external references */
int __hidden_builtin_proto1(int x) {
    (void)x;
    abort(); /* noreturn */
}

int __hidden_builtin_proto2(int x, int y) {
    return x + y;
}

int __hidden_builtin_proto3(void) {
    return global_seed;
}

int __hidden_builtin_proto4(void) {
    return 42;
}
