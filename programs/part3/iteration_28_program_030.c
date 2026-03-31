/* Built-in function visibility test for targhooks.cc coverage */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global volatile variable to prevent optimization */
volatile int global_seed = 0;

/* Function to create runtime-dependent condition */
static int get_runtime_value(void) {
    return global_seed ^ (int)(__builtin_return_address(0) & 0xFF);
}

/* ============================================================
   PROTOTYPES WITH VISIBILITY ATTRIBUTES
   These should be processed as built-in declarations
   ============================================================ */

/* Prototype 1: Full attribute combination matching target block */
extern int __hidden_builtin_1(int) 
    __attribute__((visibility("hidden"), 
                   used, 
                   artificial, 
                   noinline, 
                   noreturn));

/* Prototype 2: Variation with different attribute order */
int __hidden_builtin_2(void) 
    __attribute__((extern, 
                   visibility("hidden"), 
                   used, 
                   artificial));

/* Prototype 3: Minimal attributes but hidden visibility */
extern void __hidden_builtin_3(long) 
    __attribute__((visibility("hidden")));

/* Prototype 4: Used and artificial with hidden visibility */
int __hidden_builtin_4(int, int) 
    __attribute__((used, artificial, visibility("hidden")));

/* ============================================================
   TARGET-SPECIFIC BUILT-IN DECLARATIONS
   These will trigger target hooks for built-in processing
   ============================================================ */

#ifdef __i386__
/* x86-specific built-ins */
extern long long __builtin_ia32_rdtsc(void) 
    __attribute__((visibility("hidden"), used, artificial));

extern void __builtin_ia32_sfence(void) 
    __attribute__((visibility("hidden"), extern, used));

extern int __builtin_ia32_addcarryx_u32(unsigned char, 
                                        unsigned int, 
                                        unsigned int, 
                                        unsigned int *)
    __attribute__((visibility("hidden"), artificial));
#endif

#ifdef __x86_64__
/* x86_64-specific built-ins */
extern void __builtin_ia32_lfence(void) 
    __attribute__((visibility("hidden"), extern, used, artificial));

extern unsigned long long __builtin_ia32_rdtscp(unsigned int*) 
    __attribute__((visibility("hidden"), used));
#endif

#ifdef __arm__
/* ARM-specific built-ins */
extern unsigned int __builtin_arm_rbit(unsigned int) 
    __attribute__((visibility("hidden"), extern, used, artificial));

extern void __builtin_arm_dsb(unsigned int) 
    __attribute__((visibility("hidden"), used));
#endif

#ifdef __aarch64__
/* AArch64-specific built-ins */
extern unsigned long long __builtin_aarch64_rdtsc(void) 
    __attribute__((visibility("hidden"), extern, used, artificial));
#endif

/* ============================================================
   VOLATILE FUNCTION POINTERS
   Prevent optimization of built-in references
   ============================================================ */

/* Array of volatile function pointers */
typedef void (*func_ptr_t)(void);
static volatile func_ptr_t volatile_funcs[8];

/* Volatile pointer for single built-in */
static volatile void (*volatile_builtin_ptr)(void) = 0;

/* ============================================================
   MAIN FUNCTION WITH RUNTIME-DEPENDENT LOGIC
   ============================================================ */

int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Initialize global seed from argv if available */
    if (argc > 1) {
        global_seed = atoi(argv[1]);
    } else {
        global_seed = get_runtime_value();
    }
    
    /* Store addresses of built-ins in volatile pointers */
#ifdef __i386__
    volatile_funcs[0] = (func_ptr_t)__builtin_ia32_rdtsc;
    volatile_funcs[1] = (func_ptr_t)__builtin_ia32_sfence;
    volatile_builtin_ptr = (void (*)(void))__builtin_ia32_rdtsc;
#endif

#ifdef __x86_64__
    volatile_funcs[2] = (func_ptr_t)__builtin_ia32_lfence;
    volatile_funcs[3] = (func_ptr_t)__builtin_ia32_rdtscp;
    volatile_builtin_ptr = (void (*)(void))__builtin_ia32_lfence;
#endif

#ifdef __arm__
    volatile_funcs[4] = (func_ptr_t)__builtin_arm_rbit;
    volatile_funcs[5] = (func_ptr_t)__builtin_arm_dsb;
    volatile_builtin_ptr = (void (*)(void))__builtin_arm_rbit;
#endif

#ifdef __aarch64__
    volatile_funcs[6] = (func_ptr_t)__builtin_aarch64_rdtsc;
    volatile_builtin_ptr = (void (*)(void))__builtin_aarch64_rdtsc;
#endif
    
    /* Store prototype addresses too */
    volatile_funcs[7] = (func_ptr_t)__hidden_builtin_1;
    
    /* Create runtime-dependent condition that can't be optimized away */
    int condition = get_runtime_value() & 0x7;
    
    /* Opaque pointer comparison that forces compiler to process declarations */
    for (int i = 0; i < 8; i++) {
        if (volatile_funcs[i] != 0) {
            /* This comparison forces the compiler to materialize the address */
            if ((condition & (1 << i)) && volatile_funcs[i] == volatile_builtin_ptr) {
                result |= (1 << i);
            }
        }
    }
    
    /* Prevent dead code elimination */
    if (result == 0) {
        /* Access volatile pointer to ensure it's not optimized out */
        volatile int dummy = (volatile_builtin_ptr != 0);
        result = dummy;
    }
    
    /* Additional opaque use of built-in prototypes */
    void* addr_array[] = {
        (void*)__hidden_builtin_1,
        (void*)__hidden_builtin_2,
        (void*)__hidden_builtin_3,
        (void*)__hidden_builtin_4
    };
    
    /* Force processing of all prototype addresses */
    for (size_t i = 0; i < sizeof(addr_array)/sizeof(addr_array[0]); i++) {
        if ((uintptr_t)addr_array[i] & 0x1) {
            result++;
        }
    }
    
    return result & 0xFF;
}

/* ============================================================
   DUMMY IMPLEMENTATIONS (to satisfy linker if needed)
   These won't be called if built-ins are properly recognized
   ============================================================ */

int __hidden_builtin_1(int x) {
    /* This should never be called if GCC recognizes it as a built-in */
    abort();
    return 0;
}

int __hidden_builtin_2(void) {
    abort();
    return 0;
}

void __hidden_builtin_3(long x) {
    abort();
}

int __hidden_builtin_4(int a, int b) {
    abort();
    return a + b;
}
