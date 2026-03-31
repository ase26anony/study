/* Compile with: gcc -O0 -march=native -fdump-tree-original -fvisibility=hidden builtin_test.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global volatile variable to prevent optimization */
volatile int global_seed = 0;

/* Function to create input-dependent condition */
static int get_input_value(int argc, char **argv) {
    if (argc > 1) {
        return atoi(argv[1]);
    }
    return 42; /* Default value */
}

/* ============================================= */
/* DECLARATION OF PROTOTYPES WITH VARIOUS ATTRIBUTES */
/* ============================================= */

/* Prototype 1: Full attribute combination targeting the uncovered block */
int __hidden_builtin_1(int x) 
    __attribute__((visibility("hidden"), extern, used, artificial));

/* Prototype 2: Hidden visibility with used attribute */
int __hidden_builtin_2(int x, int y) 
    __attribute__((visibility("hidden"), used));

/* Prototype 3: Hidden visibility with artificial */
int __hidden_builtin_3(void) 
    __attribute__((visibility("hidden"), artificial, noinline));

/* Prototype 4: Just hidden visibility (should still trigger visibility processing) */
int __hidden_builtin_4(int *ptr) 
    __attribute__((visibility("hidden")));

/* Prototype 5: Extern with hidden visibility */
extern int __hidden_builtin_5(long x) 
    __attribute__((visibility("hidden"), extern));

/* ============================================= */
/* TARGET-SPECIFIC BUILT-IN DECLARATIONS */
/* ============================================= */

/* x86/x86_64 specific built-ins */
#if defined(__i386__) || defined(__x86_64__) || defined(__amd64__)

/* Declare actual x86 built-ins with hidden visibility */
int __builtin_ia32_addss(int a, int b) 
    __attribute__((visibility("hidden"), used, artificial));

int __builtin_ia32_mulss(int a, int b) 
    __attribute__((visibility("hidden"), artificial));

unsigned int __builtin_ia32_rdtsc(void) 
    __attribute__((visibility("hidden"), used));

void __builtin_ia32_sfence(void) 
    __attribute__((visibility("hidden"), artificial, extern));

/* Function that uses x86 built-ins */
static int use_x86_builtins(int x) {
    volatile int result = 0;
    
    /* Use volatile function pointers to prevent optimization */
    volatile int (*fp1)(int, int) = (int (*)(int, int))__builtin_ia32_addss;
    volatile int (*fp2)(int, int) = (int (*)(int, int))__builtin_ia32_mulss;
    volatile unsigned int (*fp3)(void) = (unsigned int (*)(void))__builtin_ia32_rdtsc;
    volatile void (*fp4)(void) = (void (*)(void))__builtin_ia32_sfence;
    
    /* Create input-dependent condition */
    if (x > 100) {
        result = fp1(x, 10);
    } else {
        result = fp2(x, 20);
    }
    
    /* Force reference to all function pointers */
    if (fp3() > 1000) {
        fp4();
    }
    
    return result;
}

#define USE_TARGET_BUILTINS use_x86_builtins

/* ARM specific built-ins */
#elif defined(__arm__) || defined(__aarch64__) || defined(__ARM_ARCH)

int __builtin_arm_rbit(int x) 
    __attribute__((visibility("hidden"), used, artificial));

int __builtin_arm_clz(int x) 
    __attribute__((visibility("hidden"), artificial));

void __builtin_arm_dmb(void) 
    __attribute__((visibility("hidden"), used, extern));

static int use_arm_builtins(int x) {
    volatile int result = 0;
    volatile int (*fp1)(int) = (int (*)(int))__builtin_arm_rbit;
    volatile int (*fp2)(int) = (int (*)(int))__builtin_arm_clz;
    volatile void (*fp3)(void) = (void (*)(void))__builtin_arm_dmb;
    
    if (x % 2 == 0) {
        result = fp1(x);
    } else {
        result = fp2(x);
    }
    
    if (result > 0) {
        fp3();
    }
    
    return result;
}

#define USE_TARGET_BUILTINS use_arm_builtins

/* PowerPC specific built-ins */
#elif defined(__powerpc__) || defined(__ppc__) || defined(__PPC__)

int __builtin_ppc_popcntb(int x) 
    __attribute__((visibility("hidden"), used, artificial));

int __builtin_ppc_mftb(void) 
    __attribute__((visibility("hidden"), artificial));

static int use_ppc_builtins(int x) {
    volatile int result = 0;
    volatile int (*fp1)(int) = (int (*)(int))__builtin_ppc_popcntb;
    volatile int (*fp2)(void) = (int (*)(void))__builtin_ppc_mftb;
    
    if (x > 50) {
        result = fp1(x);
    } else {
        result = fp2();
    }
    
    return result;
}

#define USE_TARGET_BUILTINS use_ppc_builtins

/* Generic fallback - use our prototypes */
#else

static int use_generic_builtins(int x) {
    volatile int result = 0;
    
    /* Create array of volatile function pointers to our prototypes */
    volatile int (*func_ptrs[5])(int) = {
        (int (*)(int))__hidden_builtin_1,
        (int (*)(int))__hidden_builtin_2,
        NULL, /* __hidden_builtin_3 has different signature */
        (int (*)(int))__hidden_builtin_4,
        (int (*)(int))__hidden_builtin_5
    };
    
    /* Use input-dependent index to prevent optimization */
    int idx = x % 4;
    if (idx >= 0 && idx < 4) {
        result = func_ptrs[idx](x);
    }
    
    return result;
}

#define USE_TARGET_BUILTINS use_generic_builtins

#endif

/* ============================================= */
/* MAIN FUNCTION WITH OPAQUE OPERATIONS */
/* ============================================= */

int main(int argc, char **argv) {
    int input_val = get_input_value(argc, argv);
    int result = 0;
    
    /* Array of function pointers with various attributes */
    volatile void *func_addresses[10];
    int addr_index = 0;
    
    /* Store addresses of our prototype functions */
    func_addresses[addr_index++] = (void *)__hidden_builtin_1;
    func_addresses[addr_index++] = (void *)__hidden_builtin_2;
    func_addresses[addr_index++] = (void *)__hidden_builtin_3;
    func_addresses[addr_index++] = (void *)__hidden_builtin_4;
    func_addresses[addr_index++] = (void *)__hidden_builtin_5;
    
    /* Loop to ensure compiler processes all declarations */
    for (int i = 0; i < addr_index; i++) {
        /* Opaque operation that can't be optimized away */
        global_seed += (long)func_addresses[i] & 0xFF;
    }
    
    /* Use target-specific built-ins */
    result = USE_TARGET_BUILTINS(input_val);
    
    /* Create complex conditional that can't be resolved at compile time */
    volatile int (*volatile_fp)(int) = (int (*)(int))__hidden_builtin_1;
    
    if (global_seed > 100 || input_val > 50) {
        result += volative_fp(input_val);
    } else {
        /* Compare against another function address */
        if (volative_fp == (int (*)(int))__hidden_builtin_2) {
            result += 1;
        }
    }
    
    /* Additional opaque use of function addresses */
    volatile int comparison_result = 0;
    for (int i = 0; i < addr_index - 1; i++) {
        if (func_addresses[i] == func_addresses[i + 1]) {
            comparison_result++;
        }
    }
    
    result += comparison_result;
    
    printf("Result: %d (seed: %d)\n", result, global_seed);
    return result & 0xFF; /* Return non-zero to ensure execution */
}

/* ============================================= */
/* DUMMY IMPLEMENTATIONS TO SATISFY LINKER */
/* ============================================= */

/* These would normally be provided by GCC as built-ins,
   but we provide weak implementations just in case */

int __hidden_builtin_1(int x) {
    return x + 1;
}

int __hidden_builtin_2(int x, int y) {
    return x + y;
}

int __hidden_builtin_3(void) {
    return global_seed;
}

int __hidden_builtin_4(int *ptr) {
    return ptr ? *ptr : 0;
}

int __hidden_builtin_5(long x) {
    return (int)(x & 0xFFFFFFFF);
}

/* Target-specific dummy implementations */
#if defined(__i386__) || defined(__x86_64__) || defined(__amd64__)
int __builtin_ia32_addss(int a, int b) { return a + b; }
int __builtin_ia32_mulss(int a, int b) { return a * b; }
unsigned int __builtin_ia32_rdtsc(void) { return 0; }
void __builtin_ia32_sfence(void) { }
#elif defined(__arm__) || defined(__aarch64__) || defined(__ARM_ARCH)
int __builtin_arm_rbit(int x) { return x; }
int __builtin_arm_clz(int x) { return __builtin_clz(x); }
void __builtin_arm_dmb(void) { }
#elif defined(__powerpc__) || defined(__ppc__) || defined(__PPC__)
int __builtin_ppc_popcntb(int x) { return __builtin_popcount(x); }
int __builtin_ppc_mftb(void) { return 0; }
#endif
