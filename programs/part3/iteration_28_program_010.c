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

/* ============================================
   PROTOTYPES WITH TARGET ATTRIBUTES
   ============================================ */

/* Prototype 1: Full attribute combination */
int __attribute__((visibility("hidden"), extern, used, artificial, noinline, noclone))
__hidden_builtin_1(int x);

/* Prototype 2: Hidden visibility with used */
int __attribute__((visibility("hidden"), used, noinline))
__hidden_builtin_2(int x, int y);

/* Prototype 3: Hidden with artificial */
int __attribute__((visibility("hidden"), artificial, noinline))
__hidden_builtin_3(void);

/* Prototype 4: Just hidden visibility */
int __attribute__((visibility("hidden"), noinline))
__hidden_builtin_4(int x);

/* Prototype 5: Hidden with extern linkage */
extern int __attribute__((visibility("hidden"), noinline))
__hidden_builtin_5(int x);

/* ============================================
   TARGET-SPECIFIC BUILT-IN DECLARATIONS
   ============================================ */

/* x86/x86_64 specific built-ins */
#if defined(__i386__) || defined(__x86_64__) || defined(__amd64__)

/* Use actual x86 built-ins that go through builtin_function_ext_scope */
int __attribute__((visibility("hidden"), used, artificial, noinline))
__builtin_ia32_rdtsc(void);

unsigned long long __attribute__((visibility("hidden"), used, artificial))
__builtin_ia32_rdtscp(unsigned int *);

int __attribute__((visibility("hidden"), artificial, extern))
__builtin_ia32_addss(int, int);

void __attribute__((visibility("hidden"), used, noinline))
__builtin_ia32_mfence(void);

unsigned int __attribute__((visibility("hidden"), artificial))
__builtin_ia32_crc32qi(unsigned int, unsigned char);

#endif

/* ARM specific built-ins */
#if defined(__arm__) || defined(__aarch64__) || defined(__ARM_ARCH)

int __attribute__((visibility("hidden"), used, artificial))
__builtin_arm_rbit(int);

void __attribute__((visibility("hidden"), artificial, extern))
__builtin_arm_dmb(void);

unsigned int __attribute__((visibility("hidden"), used))
__builtin_arm_clz(unsigned int);

#endif

/* PowerPC specific built-ins */
#if defined(__powerpc__) || defined(__ppc__) || defined(__PPC__)

int __attribute__((visibility("hidden"), artificial, used))
__builtin_ppc_mftb(void);

unsigned int __attribute__((visibility("hidden"), extern))
__builtin_ppc_popcntb(unsigned int);

#endif

/* Generic built-ins with hidden visibility */
void * __attribute__((visibility("hidden"), used, artificial))
__builtin_alloca(unsigned long);

int __attribute__((visibility("hidden"), artificial))
__builtin_constant_p(const void *);

/* ============================================
   FUNCTION POINTER ARRAY AND USAGE
   ============================================ */

/* Typedef for function pointers */
typedef int (*func_ptr_t)(int);

/* Volatile function pointer array to prevent optimization */
static volatile func_ptr_t volatile_funcs[8];

/* Opaque initialization that compiler can't analyze */
static void init_func_pointers(void) {
    int i;
    for (i = 0; i < 8; i++) {
        /* Use arithmetic to create non-deterministic pattern */
        volatile_funcs[i] = (func_ptr_t)((unsigned long)&__hidden_builtin_1 + 
                                        (i * get_runtime_value()));
    }
}

/* Function that creates complex control flow */
static int call_through_pointer(volatile func_ptr_t fp, int arg) {
    if (fp == (func_ptr_t)&__hidden_builtin_1) {
        return 1;
    } else if (fp == (func_ptr_t)&__hidden_builtin_2) {
        return 2;
    } else if (fp > (func_ptr_t)0x1000) {
        /* This creates a data dependency preventing optimization */
        return fp(arg + get_runtime_value());
    }
    return 0;
}

/* Main test function */
int main(int argc, char **argv) {
    int result = 0;
    volatile func_ptr_t current_fp = NULL;
    
    /* Initialize with command-line input to prevent compile-time evaluation */
    if (argc > 1) {
        global_seed = atoi(argv[1]);
    } else {
        global_seed = 12345;
    }
    
    /* Initialize function pointers */
    init_func_pointers();
    
    /* Create runtime-dependent control flow */
    for (int i = 0; i < 8; i++) {
        int runtime_val = get_runtime_value() + i;
        
        /* Complex condition that can't be optimized away */
        if ((runtime_val & 1) && (volatile_funcs[i] != NULL)) {
            current_fp = volatile_funcs[i];
            
            /* Call through volatile pointer */
            if (current_fp) {
                result += call_through_pointer(current_fp, runtime_val);
            }
        }
    }
    
    /* Use target-specific built-ins based on architecture */
    #if defined(__i386__) || defined(__x86_64__)
    {
        unsigned int aux;
        unsigned long long tsc = __builtin_ia32_rdtscp(&aux);
        result ^= (int)(tsc & 0xFFFFFFFF);
        __builtin_ia32_mfence();
    }
    #elif defined(__arm__) || defined(__aarch64__)
    {
        result ^= __builtin_arm_rbit(result);
    }
    #elif defined(__powerpc__) || defined(__ppc__)
    {
        result ^= __builtin_ppc_mftb();
    }
    #endif
    
    /* Use generic built-ins */
    void *ptr = __builtin_alloca(16);
    if (__builtin_constant_p(ptr)) {
        result += 100;
    }
    
    /* Final opaque computation */
    result = (result * 1103515245 + 12345) & 0x7fffffff;
    
    printf("Result: %d\n", result);
    return result & 1;
}

/* ============================================
   DUMMY IMPLEMENTATIONS (to satisfy linker)
   ============================================ */

int __hidden_builtin_1(int x) { return x * 2; }
int __hidden_builtin_2(int x, int y) { return x + y; }
int __hidden_builtin_3(void) { return global_seed; }
int __hidden_builtin_4(int x) { return x ^ 0x55; }
int __hidden_builtin_5(int x) { return x * x; }

/* x86 implementations */
#if defined(__i386__) || defined(__x86_64__)
int __builtin_ia32_rdtsc(void) { return 0; }
unsigned long long __builtin_ia32_rdtscp(unsigned int *aux) { *aux = 0; return 0; }
int __builtin_ia32_addss(int a, int b) { return a + b; }
void __builtin_ia32_mfence(void) { asm volatile("" ::: "memory"); }
unsigned int __builtin_ia32_crc32qi(unsigned int crc, unsigned char v) { return crc ^ v; }
#endif

/* ARM implementations */
#if defined(__arm__) || defined(__aarch64__)
int __builtin_arm_rbit(int x) { 
    int result = 0;
    for (int i = 0; i < 32; i++) {
        if (x & (1 << i)) result |= 1 << (31 - i);
    }
    return result;
}
void __builtin_arm_dmb(void) { asm volatile("" ::: "memory"); }
unsigned int __builtin_arm_clz(unsigned int x) {
    return __builtin_clz(x);
}
#endif

/* PowerPC implementations */
#if defined(__powerpc__) || defined(__ppc__)
int __builtin_ppc_mftb(void) { return 0; }
unsigned int __builtin_ppc_popcntb(unsigned int x) { return __builtin_popcount(x); }
#endif
