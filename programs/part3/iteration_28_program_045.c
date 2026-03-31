/* Built-in function visibility test for targhooks.cc coverage */
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

/* Prototype 1: Full attribute combination */
extern int __hidden_builtin_1(int) 
    __attribute__((visibility("hidden"), 
                   used, 
                   artificial, 
                   noinline, 
                   noreturn));

/* Prototype 2: Different attribute order */
int __hidden_builtin_2(float) 
    __attribute__((extern, 
                   visibility("hidden"), 
                   used, 
                   artificial));

/* Prototype 3: Minimal attributes */
extern void __hidden_builtin_3(void) 
    __attribute__((visibility("hidden")));

/* Prototype 4: With volatile parameter */
extern volatile int* __hidden_builtin_4(volatile int*) 
    __attribute__((visibility("hidden"), 
                   used, 
                   artificial));

/* ============================================
   TARGET-SPECIFIC BUILT-IN DECLARATIONS
   ============================================ */

/* X86/X86_64 specific built-ins */
#if defined(__i386__) || defined(__x86_64__) || defined(__i686__)

/* Use actual GCC x86 built-ins */
extern long long __builtin_ia32_rdtsc(void) 
    __attribute__((visibility("hidden"), 
                   used, 
                   artificial));

extern void __builtin_ia32_sfence(void) 
    __attribute__((visibility("hidden"), 
                   extern, 
                   used));

extern unsigned char __builtin_ia32_addcarryx_u32(unsigned char, 
                                                  unsigned int, 
                                                  unsigned int, 
                                                  unsigned int*) 
    __attribute__((visibility("hidden"), 
                   artificial));

/* Function to force processing of x86 built-ins */
static void use_x86_builtins(void) {
    volatile unsigned int carry = 0;
    volatile unsigned int result;
    
    /* Use volatile function pointers */
    volatile unsigned long long (*volatile rdtsc_ptr)(void) = 
        (unsigned long long (*)(void))__builtin_ia32_rdtsc;
    
    volatile void (*volatile sfence_ptr)(void) = 
        (void (*)(void))__builtin_ia32_sfence;
    
    /* Prevent dead code elimination */
    if (get_input_value() > 0) {
        unsigned long long tsc = rdtsc_ptr();
        global_seed = (int)(tsc & 0xFFFFFFFF);
    }
    
    /* Opaque use */
    sfence_ptr();
}

#endif /* x86 */

/* ARM specific built-ins */
#if defined(__arm__) || defined(__aarch64__) || defined(__thumb__)

extern unsigned int __builtin_arm_rbit(unsigned int) 
    __attribute__((visibility("hidden"), 
                   extern, 
                   used, 
                   artificial));

extern void __builtin_arm_dsb(unsigned int) 
    __attribute__((visibility("hidden"), 
                   used));

/* Function to force processing of ARM built-ins */
static void use_arm_builtins(void) {
    volatile unsigned int (*volatile rbit_ptr)(unsigned int) = 
        (unsigned int (*)(unsigned int))__builtin_arm_rbit;
    
    volatile void (*volatile dsb_ptr)(unsigned int) = 
        (void (*)(unsigned int))__builtin_arm_dsb;
    
    if (get_input_value() != 0) {
        unsigned int val = rbit_ptr(0x12345678);
        global_seed = (val & 0xFF);
    }
    
    dsb_ptr(0xF);
}

#endif /* ARM */

/* Generic built-in fallback */
#if !defined(__i386__) && !defined(__x86_64__) && \
    !defined(__arm__) && !defined(__aarch64__)

/* Use standard GCC built-ins with hidden visibility */
extern void* __builtin_return_address(unsigned int) 
    __attribute__((visibility("hidden"), 
                   extern, 
                   used, 
                   artificial));

extern int __builtin_popcount(unsigned int) 
    __attribute__((visibility("hidden"), 
                   used));

static void use_generic_builtins(void) {
    volatile void* (*volatile retaddr_ptr)(unsigned int) = 
        (void* (*)(unsigned int))__builtin_return_address;
    
    volatile int (*volatile popcount_ptr)(unsigned int) = 
        (int (*)(unsigned int))__builtin_popcount;
    
    if (get_input_value() < 100) {
        void* addr = retaddr_ptr(0);
        int count = popcount_ptr((unsigned int)(long)addr);
        global_seed = count;
    }
}

#endif /* Generic */

/* ============================================
   FUNCTION POINTER ARRAY FOR OPAQUE PROCESSING
   ============================================ */

/* Typedef for function pointers */
typedef int (*func_ptr_t)(int);

/* Array of function pointers with different attributes */
static volatile func_ptr_t func_array[] = {
    (func_ptr_t)__hidden_builtin_1,
    (func_ptr_t)__hidden_builtin_2,
    NULL, /* Placeholder for target-specific built-in */
    (func_ptr_t)__hidden_builtin_4
};

/* ============================================
   MAIN FUNCTION
   ============================================ */

int main(int argc, char *argv[]) {
    /* Initialize seed from argv to create input-dependent condition */
    if (argc > 1) {
        global_seed = atoi(argv[1]);
    } else {
        global_seed = 42;
    }
    
    /* Force processing of target-specific built-ins */
#if defined(__i386__) || defined(__x86_64__) || defined(__i686__)
    use_x86_builtins();
    /* Add x86 built-in to array */
    func_array[2] = (func_ptr_t)__builtin_ia32_rdtsc;
#elif defined(__arm__) || defined(__aarch64__) || defined(__thumb__)
    use_arm_builtins();
    func_array[2] = (func_ptr_t)__builtin_arm_rbit;
#else
    use_generic_builtins();
    func_array[2] = (func_ptr_t)__builtin_popcount;
#endif
    
    /* Loop through function pointers for opaque processing */
    volatile int result = 0;
    for (int i = 0; i < sizeof(func_array)/sizeof(func_array[0]); i++) {
        if (func_array[i] != NULL) {
            /* Create non-optimizable condition */
            volatile int input = get_input_value() + i;
            
            /* Compare function pointer addresses (can't be optimized away) */
            volatile func_ptr_t current = func_array[i];
            volatile func_ptr_t next = (i + 1 < sizeof(func_array)/sizeof(func_array[0])) ? 
                                       func_array[i + 1] : NULL;
            
            if (current != next) {
                /* This comparison forces the compiler to process the
                   function declarations and their attributes */
                result += (int)(long)current;
            }
        }
    }
    
    /* Use result in output to prevent elimination */
    printf("Result: %d (Seed: %d)\n", result, global_seed);
    
    return result != 0 ? 0 : 1;
}

/* ============================================
   DUMMY DEFINITIONS (to satisfy linker if needed)
   ============================================ */

/* These won't be called if built-ins are properly recognized */
int __hidden_builtin_1(int x) {
    fprintf(stderr, "Should not reach __hidden_builtin_1\n");
    abort();
    return 0;
}

int __hidden_builtin_2(float x) {
    fprintf(stderr, "Should not reach __hidden_builtin_2\n");
    abort();
    return 0;
}

void __hidden_builtin_3(void) {
    fprintf(stderr, "Should not reach __hidden_builtin_3\n");
    abort();
}

volatile int* __hidden_builtin_4(volatile int* x) {
    fprintf(stderr, "Should not reach __hidden_builtin_4\n");
    abort();
    return NULL;
}
