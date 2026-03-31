/* Built-in function visibility test targeting targhooks.cc lines 981-990 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global volatile variable to prevent compile-time optimization */
volatile int global_seed = 0;

/* Function to create runtime-dependent condition */
static int get_runtime_value(void) {
    return global_seed ^ (int)(__builtin_return_address(0));
}

/* ============================================
   DECLARATION OF PROTOTYPES WITH TARGET ATTRIBUTES
   These should trigger built-in processing paths
   ============================================ */

/* Prototype 1: Full attribute combination matching target block */
extern int __hidden_builtin_1(int) 
    __attribute__((visibility("hidden"), 
                   used, 
                   artificial, 
                   noinline, 
                   noreturn));

/* Prototype 2: Variation with extern explicitly specified */
extern int __hidden_builtin_2(int, int) 
    __attribute__((visibility("hidden"), 
                   extern, 
                   used, 
                   artificial));

/* Prototype 3: Without artificial but with other flags */
extern int __hidden_builtin_3(void) 
    __attribute__((visibility("hidden"), 
                   used, 
                   noinline));

/* Prototype 4: With volatile specification in attributes */
extern int __hidden_builtin_4(float) 
    __attribute__((visibility("hidden"), 
                   used, 
                   artificial, 
                   extern));

/* ============================================
   TARGET-SPECIFIC BUILT-IN DECLARATIONS
   These will go through target hooks
   ============================================ */

#if defined(__i386__) || defined(__x86_64__)
/* x86/x86-64 specific built-ins */
extern long long __builtin_ia32_rdtsc(void) 
    __attribute__((visibility("hidden"), 
                   used, 
                   artificial, 
                   extern));

extern void __builtin_ia32_sfence(void) 
    __attribute__((visibility("hidden"), 
                   used, 
                   artificial));

extern int __builtin_ia32_addcarryx_u32(unsigned char, 
                                        unsigned int, 
                                        unsigned int, 
                                        unsigned int *)
    __attribute__((visibility("hidden"), 
                   used, 
                   artificial, 
                   extern));

#elif defined(__arm__) || defined(__aarch64__)
/* ARM/AArch64 specific built-ins */
extern unsigned int __builtin_arm_rbit(unsigned int)
    __attribute__((visibility("hidden"), 
                   used, 
                   artificial, 
                   extern));

extern void __builtin_arm_dsb(unsigned int)
    __attribute__((visibility("hidden"), 
                   used, 
                   artificial));

extern unsigned long long __builtin_arm_rbitll(unsigned long long)
    __attribute__((visibility("hidden"), 
                   used, 
                   artificial, 
                   extern));

#elif defined(__powerpc__) || defined(__ppc__) || defined(__PPC__)
/* PowerPC specific built-ins */
extern unsigned int __builtin_ppc_mftb(void)
    __attribute__((visibility("hidden"), 
                   used, 
                   artificial, 
                   extern));

extern void __builtin_ppc_sync(void)
    __attribute__((visibility("hidden"), 
                   used, 
                   artificial));

#else
/* Generic fallback built-in */
extern void *__builtin_return_address(unsigned int)
    __attribute__((visibility("hidden"), 
                   used, 
                   artificial, 
                   extern));
#endif

/* ============================================
   VOLATILE FUNCTION POINTERS
   Prevent optimization of built-in references
   ============================================ */

/* Typedef for function pointers */
typedef int (*func_ptr_int_t)(int);
typedef void (*func_ptr_void_t)(void);
typedef long long (*func_ptr_ll_t)(void);

/* Volatile function pointers - compiler can't optimize these away */
volatile func_ptr_int_t volatile_fp1 = (func_ptr_int_t)0;
volatile func_ptr_void_t volatile_fp2 = (func_ptr_void_t)0;
volatile func_ptr_ll_t volatile_fp3 = (func_ptr_ll_t)0;

/* Array of volatile function pointers for loop processing */
volatile void* volatile_func_ptrs[8];

/* ============================================
   MAIN FUNCTION WITH RUNTIME-DEPENDENT LOGIC
   ============================================ */

int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Initialize global_seed from argv if available */
    if (argc > 1) {
        global_seed = atoi(argv[1]);
    } else {
        global_seed = 12345;
    }
    
    /* ============================================
       ASSIGN BUILT-IN ADDRESSES TO VOLATILE POINTERS
       This forces compiler to process the declarations
       ============================================ */
    
    /* Assign addresses - these assignments should not be optimized away */
    volatile_fp1 = (func_ptr_int_t)&__hidden_builtin_1;
    
#if defined(__i386__) || defined(__x86_64__)
    volatile_fp2 = (func_ptr_void_t)&__builtin_ia32_sfence;
    volatile_fp3 = (func_ptr_ll_t)&__builtin_ia32_rdtsc;
#elif defined(__arm__) || defined(__aarch64__)
    volatile_fp2 = (func_ptr_void_t)&__builtin_arm_dsb;
    volatile_fp3 = (func_ptr_ll_t)0; /* Placeholder - rbit returns int not ll */
#elif defined(__powerpc__) || defined(__ppc__) || defined(__PPC__)
    volatile_fp2 = (func_ptr_void_t)&__builtin_ppc_sync;
    volatile_fp3 = (func_ptr_ll_t)0; /* Placeholder */
#else
    volatile_fp2 = (func_ptr_void_t)&__builtin_return_address;
    volatile_fp3 = (func_ptr_ll_t)0;
#endif
    
    /* ============================================
       POPULATE FUNCTION POINTER ARRAY
       Multiple entries increase processing paths
       ============================================ */
    
    volatile_func_ptrs[0] = (void*)volatile_fp1;
    volatile_func_ptrs[1] = (void*)volatile_fp2;
    volatile_func_ptrs[2] = (void*)volatile_fp3;
    volatile_func_ptrs[3] = (void*)&__hidden_builtin_2;
    volatile_func_ptrs[4] = (void*)&__hidden_builtin_3;
    volatile_func_ptrs[5] = (void*)&__hidden_builtin_4;
    
    /* ============================================
       RUNTIME-DEPENDENT CONDITIONAL LOGIC
       Compiler cannot resolve these at compile-time
       ============================================ */
    
    int runtime_value = get_runtime_value();
    
    /* Complex condition that can't be optimized away */
    if ((runtime_value & 0xFF) > 128) {
        /* This branch references built-ins through volatile pointers */
        if (volatile_fp1 != (func_ptr_int_t)0) {
            /* Force reference to the built-in */
            result = (int)(long)volatile_fp1;
        }
        
        if (volatile_fp2 != (func_ptr_void_t)0) {
            result ^= (int)(long)volatile_fp2;
        }
        
        if (volatile_fp3 != (func_ptr_ll_t)0) {
            result ^= (int)(long)volatile_fp3;
        }
    } else {
        /* Alternative path with different built-in references */
        for (int i = 0; i < 6; i++) {
            if (volatile_func_ptrs[i] != 0) {
                result += (int)(long)volatile_func_ptrs[i];
            }
        }
    }
    
    /* ============================================
       ADDITIONAL OPAQUE USE OF FUNCTION POINTERS
       ============================================ */
    
    /* Loop that processes function pointers in a way the compiler
       can't analyze completely */
    for (int i = 0; i < 100; i++) {
        if ((runtime_value + i) % 7 == 0) {
            /* Opaque operation on function pointers */
            volatile_func_ptrs[i % 6] = (void*)((long)volatile_func_ptrs[i % 6] + 1);
        }
    }
    
    /* Final result depends on all the above */
    return result & 0xFF;
}

/* ============================================
   DUMMY IMPLEMENTATIONS (if needed)
   These might be needed to avoid linker errors,
   but the goal is for GCC to treat them as built-ins
   ============================================ */

/* Weak definitions that won't override built-ins */
__attribute__((weak)) int __hidden_builtin_1(int x) { return x + 1; }
__attribute__((weak)) int __hidden_builtin_2(int x, int y) { return x + y; }
__attribute__((weak)) int __hidden_builtin_3(void) { return global_seed; }
__attribute__((weak)) int __hidden_builtin_4(float x) { return (int)x; }
