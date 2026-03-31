/* Built-in function test to trigger target hook visibility handling */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global volatile variable to prevent optimization */
volatile int global_seed = 0;

/* Function to create runtime-dependent condition */
static int get_runtime_value(void) {
    return global_seed ^ (int)(__builtin_return_address(0) & 0xFF);
}

/* ================================================================
   DECLARE PROTOTYPES WITH VARIOUS ATTRIBUTE COMBINATIONS
   These should mimic built-in function declarations
   ================================================================ */

/* Prototype 1: Full attribute set matching target block */
int __hidden_builtin_1(int) 
    __attribute__((visibility("hidden"), extern, used, artificial));

/* Prototype 2: Hidden visibility with extern */
int __hidden_builtin_2(int, int) 
    __attribute__((visibility("hidden"), extern));

/* Prototype 3: Hidden with used */
int __hidden_builtin_3(void) 
    __attribute__((visibility("hidden"), used, artificial));

/* Prototype 4: Just hidden visibility */
int __hidden_builtin_4(float) 
    __attribute__((visibility("hidden")));

/* Prototype 5: Hidden with noinline to ensure address is taken */
int __hidden_builtin_5(double) 
    __attribute__((visibility("hidden"), noinline, extern, used));

/* ================================================================
   TARGET-SPECIFIC BUILT-IN DECLARATIONS
   These will trigger TARGET_BUILTIN_DECL hooks
   ================================================================ */

#if defined(__i386__) || defined(__x86_64__)
/* x86/x86-64 specific built-ins */
int __builtin_ia32_rdtsc(void) 
    __attribute__((visibility("hidden"), extern, used, artificial));

unsigned int __builtin_ia32_crc32qi(unsigned int, unsigned char)
    __attribute__((visibility("hidden"), extern, used));

int __builtin_ia32_addcarryx_u32(unsigned char, unsigned int, unsigned int, unsigned int *)
    __attribute__((visibility("hidden"), extern));

#elif defined(__arm__) || defined(__aarch64__)
/* ARM/AArch64 specific built-ins */
unsigned int __builtin_arm_rbit(unsigned int)
    __attribute__((visibility("hidden"), extern, used, artificial));

int __builtin_arm_clz(int)
    __attribute__((visibility("hidden"), extern));

unsigned int __builtin_arm_rev(unsigned int)
    __attribute__((visibility("hidden"), extern, used));

#elif defined(__powerpc__) || defined(__PPC__)
/* PowerPC specific built-ins */
int __builtin_ppc_mtfsf(int, int)
    __attribute__((visibility("hidden"), extern, used, artificial));

unsigned long long __builtin_ppc_mftb(void)
    __attribute__((visibility("hidden"), extern));

#else
/* Generic fallback - use GCC builtins with hidden visibility */
void *__builtin_frame_address(int) 
    __attribute__((visibility("hidden"), extern, used, artificial));

int __builtin_popcount(unsigned int)
    __attribute__((visibility("hidden"), extern, used));
#endif

/* ================================================================
   VOLATILE FUNCTION POINTERS TO PREVENT OPTIMIZATION
   ================================================================ */

/* Typedef for function pointers */
typedef int (*func_ptr_int_t)(int);
typedef int (*func_ptr_void_t)(void);
typedef unsigned int (*func_ptr_uint_t)(unsigned int);

/* Volatile function pointer array */
volatile func_ptr_int_t volatile_funcs[5];
volatile func_ptr_void_t volatile_void_funcs[3];

/* ================================================================
   HELPER FUNCTIONS TO USE BUILT-INS
   ================================================================ */

/* Function that takes address of built-ins */
static void take_builtin_addresses(void) {
    /* Take addresses of our declared prototypes */
    volatile_funcs[0] = (func_ptr_int_t)__hidden_builtin_1;
    volatile_funcs[1] = (func_ptr_int_t)__hidden_builtin_2;
    volatile_funcs[2] = (func_ptr_int_t)__hidden_builtin_3;
    volatile_funcs[3] = (func_ptr_int_t)__hidden_builtin_4;
    volatile_funcs[4] = (func_ptr_int_t)__hidden_builtin_5;
    
#if defined(__i386__) || defined(__x86_64__)
    /* x86 built-ins */
    volatile_void_funcs[0] = (func_ptr_void_t)__builtin_ia32_rdtsc;
    volatile_funcs[1] = (func_ptr_int_t)__builtin_ia32_crc32qi;
#elif defined(__arm__) || defined(__aarch64__)
    /* ARM built-ins */
    volatile_void_funcs[0] = (func_ptr_void_t)__builtin_arm_rbit;
    volatile_funcs[1] = (func_ptr_int_t)__builtin_arm_clz;
#elif defined(__powerpc__) || defined(__PPC__)
    /* PowerPC built-ins */
    volatile_void_funcs[0] = (func_ptr_void_t)__builtin_ppc_mtfsf;
    volatile_funcs[1] = (func_ptr_int_t)__builtin_ppc_mftb;
#else
    /* Generic built-ins */
    volatile_void_funcs[0] = (func_ptr_void_t)__builtin_frame_address;
    volatile_funcs[1] = (func_ptr_int_t)__builtin_popcount;
#endif
}

/* Opaque operation that compiler can't optimize away */
static int perform_opaque_operations(int input) {
    int result = input;
    volatile int i;
    
    /* Loop through function pointers */
    for (i = 0; i < 5; i++) {
        if (volatile_funcs[i] != 0) {
            /* Create side-effect the compiler can't eliminate */
            result ^= (int)((long)volatile_funcs[i] & 0xFF);
        }
    }
    
    for (i = 0; i < 3; i++) {
        if (volatile_void_funcs[i] != 0) {
            result += (int)((long)volatile_void_funcs[i] & 0xFF);
        }
    }
    
    return result;
}

/* ================================================================
   MAIN FUNCTION WITH RUNTIME-DEPENDENT CONTROL FLOW
   ================================================================ */

int main(int argc, char **argv) {
    int runtime_value;
    volatile func_ptr_int_t volatile_call_ptr = 0;
    
    /* Initialize based on runtime input */
    if (argc > 1) {
        global_seed = atoi(argv[1]);
    } else {
        global_seed = 12345;
    }
    
    /* Take addresses of built-ins (triggers declaration processing) */
    take_builtin_addresses();
    
    /* Get runtime-dependent value */
    runtime_value = get_runtime_value();
    
    /* Create conditional that can't be resolved at compile time */
    if ((runtime_value & 1) == 0) {
#if defined(__i386__) || defined(__x86_64__)
        volatile_call_ptr = (func_ptr_int_t)__builtin_ia32_rdtsc;
#elif defined(__arm__) || defined(__aarch64__)
        volatile_call_ptr = (func_ptr_int_t)__builtin_arm_rbit;
#elif defined(__powerpc__) || defined(__PPC__)
        volatile_call_ptr = (func_ptr_int_t)__builtin_ppc_mtfsf;
#else
        volatile_call_ptr = (func_ptr_int_t)__builtin_frame_address;
#endif
    } else {
        volatile_call_ptr = (func_ptr_int_t)__hidden_builtin_1;
    }
    
    /* Perform opaque operations with function pointers */
    int opaque_result = perform_opaque_operations(runtime_value);
    
    /* Use the volatile function pointer in a way that can't be optimized */
    if (volatile_call_ptr != 0) {
        /* Create artificial use that depends on runtime */
        opaque_result ^= (int)((long)volatile_call_ptr & 0xFF);
        
        /* Compare against known addresses (non-optimizable) */
        if ((long)volatile_call_ptr != (long)&main) {
            opaque_result += 1;
        }
    }
    
    /* Return value depends on all the opaque operations */
    return opaque_result & 0xFF;
}

/* ================================================================
   DUMMY IMPLEMENTATIONS TO SATISFY LINKER (IF NOT BUILT-INS)
   These won't be called if the functions are recognized as built-ins
   ================================================================ */

int __hidden_builtin_1(int x) { return x ^ 0x55; }
int __hidden_builtin_2(int x, int y) { return x + y; }
int __hidden_builtin_3(void) { return global_seed; }
int __hidden_builtin_4(float x) { return (int)x; }
int __hidden_builtin_5(double x) { return (int)x; }

#if defined(__i386__) || defined(__x86_64__)
int __builtin_ia32_rdtsc(void) { return 0; }
unsigned int __builtin_ia32_crc32qi(unsigned int crc, unsigned char v) { return crc ^ v; }
int __builtin_ia32_addcarryx_u32(unsigned char c, unsigned int a, unsigned int b, unsigned int *out) { 
    *out = a + b + c; 
    return 0; 
}
#elif defined(__arm__) || defined(__aarch64__)
unsigned int __builtin_arm_rbit(unsigned int x) { return x; }
int __builtin_arm_clz(int x) { return __builtin_clz(x); }
unsigned int __builtin_arm_rev(unsigned int x) { return x; }
#elif defined(__powerpc__) || defined(__PPC__)
int __builtin_ppc_mtfsf(int a, int b) { return a ^ b; }
unsigned long long __builtin_ppc_mftb(void) { return 0; }
#endif
