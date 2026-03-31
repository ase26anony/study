/* 
 * Program to trigger built-in function declaration processing with
 * hidden visibility and external linkage flags
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global volatile variable to prevent optimization */
volatile int global_seed = 0;

/* 
 * Function prototypes with various attribute combinations
 * These mimic built-in function declarations
 */

/* Prototype 1: Full attribute set matching target block */
int __hidden_builtin_1(int) 
    __attribute__((visibility("hidden"), extern, used, artificial));

/* Prototype 2: Visibility specified with hidden */
int __hidden_builtin_2(float) 
    __attribute__((visibility("hidden"), used));

/* Prototype 3: External with hidden visibility */
int __hidden_builtin_3(void) 
    __attribute__((visibility("hidden"), extern, artificial));

/* Prototype 4: All target block attributes */
int __hidden_builtin_4(double) 
    __attribute__((visibility("hidden"), extern, used, artificial, 
                   noinline, noreturn, nothrow));

/* Target-specific built-in declarations */
#ifdef __i386__ || __x86_64__
/* x86/x86-64 specific built-ins */
int __builtin_ia32_rdtsc(void) 
    __attribute__((visibility("hidden"), extern, used, artificial));
    
int __builtin_ia32_crc32qi(int, char) 
    __attribute__((visibility("hidden"), extern, used, artificial));
    
void __builtin_ia32_clflush(const void *) 
    __attribute__((visibility("hidden"), extern, used, artificial));

#elif defined(__arm__) || defined(__aarch64__)
/* ARM/AArch64 specific built-ins */
unsigned int __builtin_arm_rbit(unsigned int) 
    __attribute__((visibility("hidden"), extern, used, artificial));
    
int __builtin_arm_clz(int) 
    __attribute__((visibility("hidden"), extern, used, artificial));

#elif defined(__powerpc__) || defined(__ppc__) || defined(__PPC__)
/* PowerPC specific built-ins */
int __builtin_ppc_mftb(void) 
    __attribute__((visibility("hidden"), extern, used, artificial));

#else
/* Generic fallback - declare as external hidden functions */
int __generic_builtin_1(void) 
    __attribute__((visibility("hidden"), extern, used, artificial));
    
int __generic_builtin_2(int) 
    __attribute__((visibility("hidden"), extern, used, artificial));
#endif

/* Array of volatile function pointers to prevent optimization */
typedef int (*func_ptr_t)(void);
volatile func_ptr_t func_ptrs[4];

/* Opaque function to create non-optimizable conditions */
static int get_runtime_value(void) {
    return global_seed ^ (int)((long)&global_seed & 0xFF);
}

/* Function that takes address of built-ins */
static void take_builtin_addresses(void) {
    /* Store addresses in volatile pointers */
#ifdef __i386__ || __x86_64__
    func_ptrs[0] = (func_ptr_t)__builtin_ia32_rdtsc;
    func_ptrs[1] = (func_ptr_t)__builtin_ia32_crc32qi;
#elif defined(__arm__) || defined(__aarch64__)
    func_ptrs[0] = (func_ptr_t)__builtin_arm_rbit;
    func_ptrs[1] = (func_ptr_t)__builtin_arm_clz;
#elif defined(__powerpc__) || defined(__ppc__) || defined(__PPC__)
    func_ptrs[0] = (func_ptr_t)__builtin_ppc_mftb;
#else
    func_ptrs[0] = (func_ptr_t)__generic_builtin_1;
    func_ptrs[1] = (func_ptr_t)__generic_builtin_2;
#endif
    
    /* Also store prototype addresses */
    func_ptrs[2] = (func_ptr_t)__hidden_builtin_3;
    func_ptrs[3] = (func_ptr_t)__hidden_builtin_4;
}

/* Function to create non-optimizable calls */
static void make_opaque_calls(int selector) {
    volatile int result = 0;
    
    /* Switch that can't be optimized away */
    switch (selector & 3) {
        case 0:
            if (func_ptrs[0]) {
                /* Create artificial dependency */
                result = global_seed + 1;
            }
            break;
        case 1:
            if (func_ptrs[1]) {
                result = global_seed * 2;
            }
            break;
        case 2:
            if (func_ptrs[2]) {
                result = global_seed | 0x55;
            }
            break;
        case 3:
            if (func_ptrs[3]) {
                result = global_seed ^ 0xAA;
            }
            break;
    }
    
    /* Use result to prevent dead code elimination */
    if (result == 0xDEADBEEF) {
        /* This should never happen, but compiler doesn't know */
        printf("Impossible!\n");
    }
}

/* Main function with runtime-dependent behavior */
int main(int argc, char *argv[]) {
    int runtime_selector;
    
    /* Initialize global seed from argv if available */
    if (argc > 1) {
        global_seed = atoi(argv[1]);
    } else {
        global_seed = 12345;
    }
    
    /* Take addresses of built-ins and prototypes */
    take_builtin_addresses();
    
    /* Create runtime-dependent selector */
    runtime_selector = get_runtime_value();
    
    /* Loop to ensure all function pointers are processed */
    for (int i = 0; i < 4; i++) {
        /* Compare function pointers - can't be optimized away */
        if (func_ptrs[i] == func_ptrs[(i + 1) % 4]) {
            /* Unlikely but possible */
            runtime_selector ^= 1;
        }
        
        /* Make opaque calls based on runtime selector */
        make_opaque_calls(runtime_selector + i);
        
        /* Modify selector to create data flow */
        runtime_selector = (runtime_selector * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    /* Final conditional that depends on all previous operations */
    if (runtime_selector > 0) {
        printf("Result: %d\n", runtime_selector);
    }
    
    return 0;
}

/* Dummy definitions to satisfy linker (won't be called if built-ins exist) */
int __hidden_builtin_1(int x) { return x * 2; }
int __hidden_builtin_2(float x) { return (int)x; }
int __hidden_builtin_3(void) { return global_seed; }
int __hidden_builtin_4(double x) { return (int)x; }

#ifdef __i386__ || __x86_64__
int __builtin_ia32_rdtsc(void) { return 0; }
int __builtin_ia32_crc32qi(int a, char b) { return a ^ b; }
void __builtin_ia32_clflush(const void *p) { (void)p; }
#elif defined(__arm__) || defined(__aarch64__)
unsigned int __builtin_arm_rbit(unsigned int x) { return x; }
int __builtin_arm_clz(int x) { return __builtin_clz(x); }
#elif defined(__powerpc__) || defined(__PPC__)
int __builtin_ppc_mftb(void) { return 0; }
#else
int __generic_builtin_1(void) { return 1; }
int __generic_builtin_2(int x) { return x; }
#endif
