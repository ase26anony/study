/* Built-in function visibility test program */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global volatile variable to prevent optimization */
volatile int global_seed = 0;

/* ============================================
   PROTOTYPES WITH VISIBILITY ATTRIBUTES
   ============================================ */

/* Prototype 1: Full attribute combination */
extern int __attribute__((visibility("hidden"), used, artificial, noinline))
__hidden_builtin_1(int x) __asm__("__hidden_builtin_1");

/* Prototype 2: Hidden visibility with extern */
extern int __attribute__((visibility("hidden")))
__hidden_builtin_2(int x, int y);

/* Prototype 3: Hidden + used + artificial */
extern int __attribute__((visibility("hidden"), used, artificial))
__hidden_builtin_3(void);

/* Prototype 4: Just hidden visibility */
int __attribute__((visibility("hidden")))
__hidden_builtin_4(long x);

/* ============================================
   TARGET-SPECIFIC BUILT-IN DECLARATIONS
   ============================================ */

#ifdef __i386__
/* x86 specific built-ins */
extern long __attribute__((visibility("hidden"), used, artificial))
__builtin_ia32_rdtsc(void);

extern void __attribute__((visibility("hidden")))
__builtin_ia32_sfence(void);

extern int __attribute__((visibility("hidden"), used))
__builtin_ia32_addcarryx_u32(unsigned char __cf, unsigned int __x,
                             unsigned int __y, unsigned int *__p);
#endif

#ifdef __x86_64__
/* x86_64 specific built-ins */
extern void __attribute__((visibility("hidden"), used, artificial))
__builtin_ia32_lfence(void);

extern unsigned long long __attribute__((visibility("hidden")))
__builtin_ia32_rdtsc(void);

extern void __attribute__((visibility("hidden"), artificial))
__builtin_ia32_mfence(void);
#endif

#ifdef __arm__
/* ARM specific built-ins */
extern unsigned int __attribute__((visibility("hidden"), used, artificial))
__builtin_arm_rbit(unsigned int x);

extern void __attribute__((visibility("hidden")))
__builtin_arm_dmb(unsigned int x);

extern unsigned int __attribute__((visibility("hidden"), used))
__builtin_arm_clz(unsigned int x);
#endif

#ifdef __aarch64__
/* AArch64 specific built-ins */
extern unsigned long long __attribute__((visibility("hidden"), used, artificial))
__builtin_aarch64_rbitdi(unsigned long long x);

extern void __attribute__((visibility("hidden")))
__builtin_aarch64_yield(void);
#endif

/* ============================================
   FUNCTION POINTER ARRAY FOR OPAQUE OPERATIONS
   ============================================ */

/* Volatile function pointer to prevent optimization */
typedef int (*func_ptr_t)(int);
volatile func_ptr_t volatile_fptr = NULL;

/* Array of function pointers for iteration */
static void* func_ptrs[8] = {0};

/* ============================================
   MAIN FUNCTION WITH NON-OPTIMIZABLE LOGIC
   ============================================ */

int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Use argv to create non-optimizable condition */
    int use_builtin = 0;
    if (argc > 1) {
        use_builtin = (argv[1][0] % 2);
        global_seed = argv[1][0];
    }
    
    /* Initialize function pointer array with addresses */
    /* The compiler should process these declarations */
    func_ptrs[0] = (void*)__hidden_builtin_1;
    func_ptrs[1] = (void*)__hidden_builtin_2;
    func_ptrs[2] = (void*)__hidden_builtin_3;
    func_ptrs[3] = (void*)__hidden_builtin_4;
    
    /* Target-specific built-in addresses */
#ifdef __i386__
    func_ptrs[4] = (void*)__builtin_ia32_rdtsc;
    func_ptrs[5] = (void*)__builtin_ia32_sfence;
#endif
    
#ifdef __x86_64__
    func_ptrs[4] = (void*)__builtin_ia32_rdtsc;
    func_ptrs[5] = (void*)__builtin_ia32_lfence;
#endif
    
#ifdef __arm__
    func_ptrs[4] = (void*)__builtin_arm_rbit;
    func_ptrs[5] = (void*)__builtin_arm_clz;
#endif
    
#ifdef __aarch64__
    func_ptrs[4] = (void*)__builtin_aarch64_rbitdi;
    func_ptrs[5] = (void*)__builtin_aarch64_yield;
#endif
    
    /* Volatile assignment to force processing */
    volatile_fptr = (func_ptr_t)func_ptrs[use_builtin % 4];
    
    /* Loop with opaque operations on function pointers */
    for (int i = 0; i < 6; i++) {
        if (func_ptrs[i]) {
            /* Create non-optimizable comparison */
            if ((unsigned long)func_ptrs[i] > (unsigned long)&global_seed) {
                result += i;
            }
        }
    }
    
    /* Conditional call through volatile pointer */
    if (use_builtin && volatile_fptr) {
        /* This should trigger built-in processing */
        result += volatile_fptr(global_seed);
    }
    
    /* Additional artificial use to ensure declarations are processed */
    __asm__ volatile ("" : : "r"(__hidden_builtin_1), "r"(__hidden_builtin_2));
    
    return result % 256;
}

/* ============================================
   DUMMY IMPLEMENTATIONS (if not built-in)
   ============================================ */

/* These provide implementations if the compiler doesn't recognize them as built-ins */
int __hidden_builtin_1(int x) {
    return x + 1;
}

int __hidden_builtin_2(int x, int y) {
    return x + y;
}

int __hidden_builtin_3(void) {
    return global_seed;
}

int __hidden_builtin_4(long x) {
    return (int)x;
}
