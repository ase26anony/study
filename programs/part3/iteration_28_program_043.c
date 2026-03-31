/* 
 * Program to trigger built-in function declaration handling with
 * hidden visibility and external linkage in GCC's targhooks.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global volatile variable to prevent compile-time optimization */
volatile int global_seed = 0;

/* 
 * Function prototypes with various attribute combinations
 * These mimic built-in function declarations
 */

/* Prototype 1: Full set of attributes matching target block */
__attribute__((visibility("hidden"), extern, used, artificial, noinline, noreturn))
void __hidden_builtin_full(void);

/* Prototype 2: Hidden visibility with external linkage */
__attribute__((visibility("hidden"), extern))
int __hidden_builtin_int(int x);

/* Prototype 3: Hidden visibility with used attribute */
__attribute__((visibility("hidden"), used))
float __hidden_builtin_float(float x);

/* Prototype 4: Artificial with hidden visibility */
__attribute__((artificial, visibility("hidden")))
double __hidden_builtin_double(double x);

/* Prototype 5: All target attributes except one */
__attribute__((visibility("hidden"), extern, used, noinline))
char __hidden_builtin_char(char x);

/* Target-specific built-in declarations */
#ifdef __i386__

/* x86-specific built-ins that should trigger TARGET_BUILTIN_DECL hook */
__attribute__((visibility("hidden"), extern, used, artificial))
int __builtin_ia32_rdtsc(void);

__attribute__((visibility("hidden"), extern, used, artificial))
void __builtin_ia32_cpuid(int regs[4], int leaf);

__attribute__((visibility("hidden"), extern, used, artificial))
unsigned long long __builtin_ia32_readeflags_u64(void);

#elif defined(__x86_64__)

/* x86_64-specific built-ins */
__attribute__((visibility("hidden"), extern, used, artificial))
unsigned long long __builtin_ia32_rdtsc(void);

__attribute__((visibility("hidden"), extern, used, artificial))
void __builtin_ia32_cpuid(int regs[4], int leaf);

__attribute__((visibility("hidden"), extern, used, artificial))
unsigned long long __builtin_ia32_readeflags_u64(void);

#elif defined(__arm__)

/* ARM-specific built-ins */
__attribute__((visibility("hidden"), extern, used, artificial))
unsigned int __builtin_arm_get_cpsr(void);

__attribute__((visibility("hidden"), extern, used, artificial))
void __builtin_arm_set_cpsr(unsigned int);

__attribute__((visibility("hidden"), extern, used, artificial))
unsigned int __builtin_arm_rbit(unsigned int);

#elif defined(__aarch64__)

/* AArch64-specific built-ins */
__attribute__((visibility("hidden"), extern, used, artificial))
unsigned long long __builtin_aarch64_get_fpcr(void);

__attribute__((visibility("hidden"), extern, used, artificial))
void __builtin_aarch64_set_fpcr(unsigned long long);

#else

/* Generic fallback - declare as weak symbols to avoid linkage errors */
__attribute__((visibility("hidden"), extern, used, artificial, weak))
int __generic_builtin_test(void);

#endif

/* Array of volatile function pointers to prevent optimization */
typedef void (*func_ptr_t)(void);
volatile func_ptr_t volatile_funcs[10];

/* Opaque operation to use function pointers without being optimized away */
static void opaque_operation(func_ptr_t func) {
    /* Use inline assembly to create a side effect */
    __asm__ volatile ("" : : "r"(func) : "memory");
}

int main(int argc, char *argv[]) {
    /* Use argv to create input-dependent behavior */
    int use_builtin = 0;
    if (argc > 1) {
        use_builtin = (argv[1][0] % 2);
    }
    
    /* Initialize volatile function pointers with various built-in addresses */
    volatile_funcs[0] = (func_ptr_t)__hidden_builtin_full;
    volatile_funcs[1] = (func_ptr_t)__hidden_builtin_int;
    volatile_funcs[2] = (func_ptr_t)__hidden_builtin_float;
    volatile_funcs[3] = (func_ptr_t)__hidden_builtin_double;
    volatile_funcs[4] = (func_ptr_t)__hidden_builtin_char;
    
    /* Target-specific built-in assignments */
#ifdef __i386__ || defined(__x86_64__)
    volatile_funcs[5] = (func_ptr_t)__builtin_ia32_rdtsc;
    volatile_funcs[6] = (func_ptr_t)__builtin_ia32_cpuid;
#elif defined(__arm__)
    volatile_funcs[5] = (func_ptr_t)__builtin_arm_get_cpsr;
    volatile_funcs[6] = (func_ptr_t)__builtin_arm_rbit;
#elif defined(__aarch64__)
    volatile_funcs[5] = (func_ptr_t)__builtin_aarch64_get_fpcr;
#else
    volatile_funcs[5] = (func_ptr_t)__generic_builtin_test;
#endif
    
    /* Create a non-optimizable conditional using volatile variables */
    volatile int condition = global_seed + use_builtin;
    func_ptr_t selected_func = NULL;
    
    /* Complex conditional that can't be resolved at compile time */
    for (int i = 0; i < 7; i++) {
        if ((condition & (1 << i)) != 0) {
            selected_func = (func_ptr_t)volatile_funcs[i];
            break;
        }
    }
    
    /* Perform opaque operations with all function pointers */
    for (int i = 0; i < 7; i++) {
        if (volatile_funcs[i] != NULL) {
            opaque_operation((func_ptr_t)volatile_funcs[i]);
        }
    }
    
    /* Non-optimizable comparison of function addresses */
    volatile func_ptr_t compare_func = (func_ptr_t)__hidden_builtin_full;
    if (selected_func == compare_func) {
        /* This branch is unpredictable at compile time */
        printf("Selected function matches hidden builtin\n");
    }
    
    /* Additional complexity to ensure processing of all declarations */
    int result = 0;
    if (use_builtin) {
        /* Call through function pointer - compiler can't optimize this away */
        if (selected_func != NULL) {
            /* Note: Actual call would need proper signature */
            printf("Function pointer selected: %p\n", (void*)selected_func);
        }
        
        /* Take addresses of all declared functions */
        void* addresses[] = {
            (void*)__hidden_builtin_full,
            (void*)__hidden_builtin_int,
            (void*)__hidden_builtin_float,
            (void*)__hidden_builtin_double,
            (void*)__hidden_builtin_char,
#ifdef __i386__ || defined(__x86_64__)
            (void*)__builtin_ia32_rdtsc,
            (void*)__builtin_ia32_cpuid,
#elif defined(__arm__)
            (void*)__builtin_arm_get_cpsr,
            (void*)__builtin_arm_rbit,
#endif
        };
        
        /* Opaque computation with addresses */
        for (size_t i = 0; i < sizeof(addresses)/sizeof(addresses[0]); i++) {
            result ^= (int)((long)addresses[i] & 0xFFFF);
        }
    }
    
    return result;
}

/* Dummy implementations to satisfy linker (won't be called in normal execution) */
void __hidden_builtin_full(void) {
    __builtin_unreachable();
}

int __hidden_builtin_int(int x) {
    return x ^ 0x55;
}

float __hidden_builtin_float(float x) {
    return x * 2.0f;
}

double __hidden_builtin_double(double x) {
    return x / 2.0;
}

char __hidden_builtin_char(char x) {
    return x + 1;
}
