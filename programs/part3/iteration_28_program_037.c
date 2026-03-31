/* Built-in function visibility test program */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global volatile variable to prevent optimization */
volatile int global_seed = 0;

/* ============================================
   DECLARATION OF PROTOTYPES WITH TARGET ATTRIBUTES
   ============================================ */

/* Prototype 1: Full attribute combination matching uncovered lines */
int __attribute__((visibility("hidden"), extern, used, artificial, noinline, noclone))
__hidden_builtin_proto1(int x);

/* Prototype 2: Different attribute order */
int __attribute__((extern, used, artificial, visibility("hidden")))
__hidden_builtin_proto2(int x);

/* Prototype 3: With nothrow and volatile attributes */
int __attribute__((visibility("hidden"), extern, used, artificial, nothrow))
__hidden_builtin_proto3(int x) __attribute__((volatile));

/* Prototype 4: Minimal attributes but hidden visibility */
int __attribute__((visibility("hidden")))
__hidden_builtin_proto4(int x);

/* ============================================
   TARGET-SPECIFIC BUILT-IN DECLARATIONS
   ============================================ */

#if defined(__i386__) || defined(__x86_64__)
/* x86/x86-64 specific built-ins */
int __attribute__((visibility("hidden"), extern, used, artificial))
__builtin_ia32_rdtsc(void);

int __attribute__((visibility("hidden"), extern, used, artificial))
__builtin_ia32_crc32qi(int crc, char v);

void __attribute__((visibility("hidden"), extern, used, artificial))
__builtin_ia32_pause(void);

/* SSE built-ins */
int __attribute__((visibility("hidden"), extern, used, artificial))
__builtin_ia32_comieq(__m128, __m128);

#elif defined(__arm__) || defined(__aarch64__)
/* ARM specific built-ins */
unsigned int __attribute__((visibility("hidden"), extern, used, artificial))
__builtin_arm_rbit(unsigned int x);

int __attribute__((visibility("visibility_hidden"), extern, used, artificial))
__builtin_arm_clz(int x);

#elif defined(__powerpc__) || defined(__ppc__) || defined(__PPC__)
/* PowerPC specific built-ins */
unsigned int __attribute__((visibility("hidden"), extern, used, artificial))
__builtin_ppc_popcntb(unsigned int x);

#else
/* Generic fallback - declare as external hidden functions */
int __attribute__((visibility("hidden"), extern, used, artificial))
__generic_hidden_builtin(int x);
#endif

/* ============================================
   FUNCTION POINTER ARRAY AND OPAQUE OPERATIONS
   ============================================ */

/* Volatile function pointer to prevent optimization */
typedef int (*volatile_func_ptr)(int) __attribute__((volatile));

/* Array of function pointers for iteration */
volatile_func_ptr func_ptrs[8];

/* Opaque initialization that compiler can't analyze */
void init_function_pointers(int use_real_builtins) {
    /* Initialize with different patterns based on input */
    if (use_real_builtins) {
        #if defined(__i386__) || defined(__x86_64__)
        /* Cast built-in functions to function pointers */
        func_ptrs[0] = (volatile_func_ptr)__builtin_ia32_rdtsc;
        func_ptrs[1] = (volatile_func_ptr)__builtin_ia32_crc32qi;
        #elif defined(__arm__) || defined(__aarch64__)
        func_ptrs[0] = (volatile_func_ptr)__builtin_arm_rbit;
        func_ptrs[1] = (volatile_func_ptr)__builtin_arm_clz;
        #else
        func_ptrs[0] = (volatile_func_ptr)__generic_hidden_builtin;
        #endif
    } else {
        func_ptrs[0] = (volatile_func_ptr)__hidden_builtin_proto1;
        func_ptrs[1] = (volatile_func_ptr)__hidden_builtin_proto2;
        func_ptrs[2] = (volatile_func_ptr)__hidden_builtin_proto3;
        func_ptrs[3] = (volatile_func_ptr)__hidden_builtin_proto4;
    }
    
    /* Fill remaining slots with alternating patterns */
    for (int i = 4; i < 8; i++) {
        func_ptrs[i] = (volatile_func_ptr)((long)func_ptrs[i-4] + global_seed);
    }
}

/* Non-optimizable comparison function */
int compare_function_pointers(void) {
    volatile static int counter = 0;
    int result = 0;
    
    for (int i = 0; i < 7; i++) {
        /* This comparison cannot be optimized away */
        if (func_ptrs[i] == func_ptrs[i+1]) {
            result++;
        }
        /* Opaque side effect */
        counter += (long)func_ptrs[i];
    }
    
    return result + counter;
}

/* ============================================
   MAIN FUNCTION WITH COMPLEX CONTROL FLOW
   ============================================ */

int main(int argc, char *argv[]) {
    int use_builtins = 0;
    volatile int input_dependent = 0;
    
    /* Use argv to create input-dependent behavior */
    if (argc > 1) {
        input_dependent = atoi(argv[1]);
        use_builtins = (input_dependent % 2) == 0;
        global_seed = input_dependent;
    }
    
    /* Initialize function pointers based on input */
    init_function_pointers(use_builtins);
    
    /* Create a non-optimizable conditional */
    volatile int condition = compare_function_pointers();
    
    int result = 0;
    
    /* Loop that forces compiler to process all function pointers */
    for (int i = 0; i < 8; i++) {
        if (condition > i) {
            /* This call may or may not happen at runtime */
            if (func_ptrs[i]) {
                /* Store result to prevent dead code elimination */
                volatile int temp = ((int (*)(int))func_ptrs[i])(i);
                result += temp;
            }
        }
    }
    
    /* Additional complexity to ensure processing */
    switch (input_dependent % 4) {
        case 0:
            #if defined(__i386__) || defined(__x86_64__)
            __builtin_ia32_pause();
            #endif
            break;
        case 1:
            result += __hidden_builtin_proto1(input_dependent);
            break;
        case 2:
            result += __hidden_builtin_proto2(input_dependent * 2);
            break;
        case 3:
            result += __hidden_builtin_proto3(input_dependent / 2);
            break;
    }
    
    /* Final opaque computation */
    result += (condition & 0xFF) | (global_seed << 8);
    
    printf("Result: %d\n", result);
    return result & 1; /* Return 0 or 1 for exit code */
}

/* ============================================
   DUMMY IMPLEMENTATIONS (NEVER CALLED AT RUNTIME)
   ============================================ */

/* These exist only to satisfy references in non-optimized paths */
int __hidden_builtin_proto1(int x) { return x + 1; }
int __hidden_builtin_proto2(int x) { return x * 2; }
int __hidden_builtin_proto3(int x) { return x - 1; }
int __hidden_builtin_proto4(int x) { return x ^ 0xFF; }

#if !(defined(__i386__) || defined(__x86_64__) || \
      defined(__arm__) || defined(__aarch64__) || \
      defined(__powerpc__) || defined(__ppc__) || defined(__PPC__))
int __generic_hidden_builtin(int x) { return x; }
#endif
