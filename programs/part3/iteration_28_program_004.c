/* Built-in function visibility test program */
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
   DECLARATION OF PROTOTYPES WITH TARGET ATTRIBUTES
   ============================================ */

/* Prototype 1: Full attribute combination */
extern int __attribute__((visibility("hidden"), used, artificial, noinline, noreturn))
    __hidden_builtin_1(int) __asm__("__hidden_builtin_1");

/* Prototype 2: Different ordering */
extern int __attribute__((used, artificial, visibility("hidden")))
    __hidden_builtin_2(void) __asm__("__hidden_builtin_2");

/* Prototype 3: With extern explicitly stated */
extern int __attribute__((extern, visibility("hidden"), used, artificial))
    __hidden_builtin_3(long) __asm__("__hidden_builtin_3");

/* Prototype 4: Minimal attributes */
extern int __attribute__((visibility("hidden")))
    __hidden_builtin_4(int, int) __asm__("__hidden_builtin_4");

/* Prototype 5: With volatile-like qualifiers */
extern int __attribute__((visibility("hidden"), used, artificial, const))
    __hidden_builtin_5(void) __asm__("__hidden_builtin_5");

/* ============================================
   TARGET-SPECIFIC BUILT-IN DECLARATIONS
   ============================================ */

/* x86/x86-64 specific built-ins */
#if defined(__i386__) || defined(__x86_64__) || defined(__i686__)

/* Use actual x86 built-ins that go through builtin_function_ext_scope */
extern long long __attribute__((visibility("hidden"), used, artificial))
    __builtin_ia32_rdtsc(void);

extern void __attribute__((visibility("hidden"), used, artificial))
    __builtin_ia32_mfence(void);

extern int __attribute__((visibility("hidden"), used, artificial))
    __builtin_ia32_addcarryx_u32(unsigned char, unsigned int, 
                                 unsigned int, unsigned int *);

extern unsigned int __attribute__((visibility("hidden"), used, artificial))
    __builtin_ia32_crc32qi(unsigned int, unsigned char);

/* Define function pointers for x86 built-ins */
typedef long long (*volatile rdtsc_ptr_t)(void);
typedef void (*volatile mfence_ptr_t)(void);

#elif defined(__arm__) || defined(__aarch64__) || defined(__thumb__)

/* ARM-specific built-ins */
extern unsigned int __attribute__((visibility("hidden"), used, artificial))
    __builtin_arm_rbit(unsigned int);

extern void __attribute__((visibility("hidden"), used, artificial))
    __builtin_arm_dmb(unsigned int);

extern unsigned int __attribute__((visibility("hidden"), used, artificial))
    __builtin_arm_clz(unsigned int);

/* Define function pointers for ARM built-ins */
typedef unsigned int (*volatile rbit_ptr_t)(unsigned int);
typedef void (*volatile dmb_ptr_t)(unsigned int);

#elif defined(__powerpc__) || defined(__ppc__) || defined(__PPC__)

/* PowerPC-specific built-ins */
extern unsigned int __attribute__((visibility("hidden"), used, artificial))
    __builtin_ppc_popcntb(unsigned int);

extern unsigned long long __attribute__((visibility("hidden"), used, artificial))
    __builtin_ppc_mftb(void);

#else
/* Generic fallback - use GCC generic built-ins */
extern void __attribute__((visibility("hidden"), used, artificial))
    __builtin_trap(void);

extern int __attribute__((visibility("hidden"), used, artificial))
    __builtin_popcount(unsigned int);

extern void __attribute__((visibility("hidden"), used, artificial))
    __builtin_prefetch(const void *, ...);

#endif

/* ============================================
   FUNCTION POINTER ARRAY AND OPAQUE OPERATIONS
   ============================================ */

/* Array of volatile function pointers to prevent optimization */
typedef int (*func_ptr_t)(int);
static volatile func_ptr_t func_array[8];

/* Opaque initialization that compiler can't analyze */
static void init_func_array(void) {
    /* Use address of built-in prototypes */
    func_array[0] = (func_ptr_t)__hidden_builtin_1;
    func_array[1] = (func_ptr_t)__hidden_builtin_2;
    func_array[2] = (func_ptr_t)__hidden_builtin_3;
    func_array[3] = (func_ptr_t)__hidden_builtin_4;
    func_array[4] = (func_ptr_t)__hidden_builtin_5;
    
    /* Leave some slots for target-specific built-ins */
#if defined(__i386__) || defined(__x86_64__)
    func_array[5] = (func_ptr_t)__builtin_ia32_crc32qi;
#elif defined(__arm__) || defined(__aarch64__)
    func_array[5] = (func_ptr_t)__builtin_arm_rbit;
#endif
    
    /* Fill rest with NULL */
    func_array[6] = 0;
    func_array[7] = 0;
}

/* Opaque operation that uses function pointers */
static int process_with_builtins(int input) {
    volatile int result = 0;
    int i;
    
    for (i = 0; i < 8; i++) {
        if (func_array[i] && (input & (1 << i))) {
            /* This call can't be optimized away due to volatile pointer */
            result += func_array[i](input + i);
        }
    }
    
    return result;
}

/* ============================================
   MAIN FUNCTION WITH RUNTIME-DEPENDENT LOGIC
   ============================================ */

int main(int argc, char *argv[]) {
    int runtime_value;
    volatile int output = 0;
    
    /* Initialize with command-line input to make value runtime-dependent */
    if (argc > 1) {
        global_seed = atoi(argv[1]);
    } else {
        global_seed = (int)__builtin_return_address(0);
    }
    
    /* Initialize function pointer array */
    init_func_array();
    
    /* Get runtime-dependent value */
    runtime_value = get_runtime_value();
    
    /* Use target-specific built-ins based on runtime conditions */
#if defined(__i386__) || defined(__x86_64__)
    {
        volatile rdtsc_ptr_t rdtsc_ptr = __builtin_ia32_rdtsc;
        volatile mfence_ptr_t mfence_ptr = __builtin_ia32_mfence;
        
        /* Non-optimizable comparison and usage */
        if (runtime_value & 1) {
            long long tsc = rdtsc_ptr();
            output += (int)(tsc & 0xFFFFFFFF);
        }
        
        if (runtime_value & 2) {
            mfence_ptr();
        }
        
        /* Use the addcarry built-in */
        unsigned int carry_out;
        unsigned char carry_in = (runtime_value >> 3) & 1;
        unsigned int a = runtime_value;
        unsigned int b = runtime_value * 3;
        
        int result = __builtin_ia32_addcarryx_u32(carry_in, a, b, &carry_out);
        output += result + carry_out;
    }
    
#elif defined(__arm__) || defined(__aarch64__)
    {
        volatile rbit_ptr_t rbit_ptr = __builtin_arm_rbit;
        volatile dmb_ptr_t dmb_ptr = __builtin_arm_dmb;
        
        if (runtime_value & 1) {
            unsigned int reversed = rbit_ptr(runtime_value);
            output += reversed;
        }
        
        if (runtime_value & 4) {
            dmb_ptr(0xF); /* Full system data memory barrier */
        }
        
        /* Use clz built-in */
        unsigned int clz_result = __builtin_arm_clz(runtime_value);
        output += clz_result;
    }
    
#else
    /* Generic path using generic built-ins */
    {
        if (runtime_value & 1) {
            output += __builtin_popcount(runtime_value);
        }
        
        /* Prefetch based on runtime value */
        void *addr = (void*)(long)runtime_value;
        __builtin_prefetch(addr, 0, 3);
    }
#endif
    
    /* Process with our function pointer array */
    output += process_with_builtins(runtime_value);
    
    /* Create a conditional that can't be resolved at compile time */
    volatile int *volatile_output = &output;
    if (*volatile_output > 1000) {
        /* This should never happen, but compiler can't know */
        __builtin_unreachable();
    }
    
    /* Use output to prevent dead code elimination */
    printf("Result: %d (seed: %d)\n", output, global_seed);
    
    return output & 0xFF;
}

/* ============================================
   DUMMY IMPLEMENTATIONS TO SATISFY LINKER
   (These won't be used if built-ins are properly recognized)
   ============================================ */

int __hidden_builtin_1(int x) {
    return x ^ 0x55AA55AA;
}

int __hidden_builtin_2(void) {
    return global_seed;
}

int __hidden_builtin_3(long x) {
    return (int)(x >> 32) ^ (int)x;
}

int __hidden_builtin_4(int a, int b) {
    return a + b;
}

int __hidden_builtin_5(void) {
    return 42;
}
