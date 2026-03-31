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

/* Prototype 1: Full attribute set matching the uncovered block */
int __hidden_builtin_1(int) 
    __attribute__((visibility("hidden"), 
                   extern, 
                   used, 
                   artificial, 
                   noinline, 
                   noreturn));

/* Prototype 2: Partial attribute set */
int __hidden_builtin_2(float, float) 
    __attribute__((visibility("hidden"), used));

/* Prototype 3: Different type signature */
void __hidden_builtin_3(void*) 
    __attribute__((visibility("hidden"), 
                   extern, 
                   artificial));

/* Prototype 4: With volatile parameters */
volatile int* __hidden_builtin_4(volatile int*, int) 
    __attribute__((visibility("hidden"), used, artificial));

/* ============================================
   TARGET-SPECIFIC BUILT-IN DECLARATIONS
   ============================================ */

#ifdef __i386__
/* x86/x86-64 specific built-ins */
int __builtin_ia32_rdtsc(void) 
    __attribute__((visibility("hidden"), used, artificial));

void __builtin_ia32_sfence(void) 
    __attribute__((visibility("hidden"), extern, used));

unsigned int __builtin_ia32_crc32qi(unsigned int, unsigned char)
    __attribute__((visibility("hidden"), artificial));

/* Store function pointers to prevent optimization */
typedef int (*volatile_func_ptr)(void);
volatile volatile_func_ptr volatile_fptr = (volatile_func_ptr)__builtin_ia32_rdtsc;

#endif

#ifdef __x86_64__
/* x86-64 specific built-ins */
unsigned long long __builtin_ia32_rdtsc64(void)
    __attribute__((visibility("hidden"), used, artificial, extern));

void __builtin_ia32_mfence(void)
    __attribute__((visibility("hidden"), used));

long __builtin_ia32_crc32di(long, long)
    __attribute__((visibility("hidden"), artificial, extern));

volatile unsigned long long (*volatile_rdtsc_ptr)(void) = __builtin_ia32_rdtsc64;
#endif

#ifdef __arm__
/* ARM specific built-ins */
unsigned int __builtin_arm_rbit(unsigned int)
    __attribute__((visibility("hidden"), used, artificial));

void __builtin_arm_dmb(unsigned int)
    __attribute__((visibility("hidden"), extern, used));

int __builtin_arm_clz(int)
    __attribute__((visibility("hidden"), artificial));

volatile unsigned int (*volatile_rbit_ptr)(unsigned int) = __builtin_arm_rbit;
#endif

#ifdef __aarch64__
/* AArch64 specific built-ins */
unsigned long long __builtin_aarch64_rbitll(unsigned long long)
    __attribute__((visibility("hidden"), used, artificial, extern));

void __builtin_aarch64_yield(void)
    __attribute__((visibility("hidden"), used));

int __builtin_aarch64_clrsb(int)
    __attribute__((visibility("hidden"), artificial));

volatile unsigned long long (*volatile_rbitll_ptr)(unsigned long long) = 
    __builtin_aarch64_rbitll;
#endif

#ifdef __powerpc__
/* PowerPC specific built-ins */
unsigned int __builtin_ppc_mftb(void)
    __attribute__((visibility("hidden"), used, artificial, extern));

int __builtin_ppc_popcntb(int)
    __attribute__((visibility("hidden"), used));

void __builtin_ppc_sync(void)
    __attribute__((visibility("hidden"), artificial, extern));

volatile unsigned int (*volatile_mftb_ptr)(void) = __builtin_ppc_mftb;
#endif

/* ============================================
   FUNCTION POINTER ARRAY FOR OPAQUE OPERATIONS
   ============================================ */

/* Generic function pointer type */
typedef void (*generic_func_ptr)(void);

/* Array of function pointers for iteration */
static generic_func_ptr func_array[] = {
    (generic_func_ptr)__hidden_builtin_1,
    (generic_func_ptr)__hidden_builtin_2,
    (generic_func_ptr)__hidden_builtin_3,
    (generic_func_ptr)__hidden_builtin_4,
#ifdef __i386__
    (generic_func_ptr)__builtin_ia32_rdtsc,
    (generic_func_ptr)__builtin_ia32_sfence,
#endif
#ifdef __x86_64__
    (generic_func_ptr)__builtin_ia32_rdtsc64,
    (generic_func_ptr)__builtin_ia32_mfence,
#endif
#ifdef __arm__
    (generic_func_ptr)__builtin_arm_rbit,
    (generic_func_ptr)__builtin_arm_dmb,
#endif
    NULL
};

/* ============================================
   MAIN FUNCTION WITH NON-OPTIMIZABLE LOGIC
   ============================================ */

int main(int argc, char **argv) {
    int result = 0;
    volatile int condition;
    
    /* Use argv to create runtime-dependent condition */
    if (argc > 1) {
        condition = atoi(argv[1]);
    } else {
        condition = get_runtime_value();
    }
    
    /* Volatile function pointer to prevent optimization */
    generic_func_ptr volatile volatile_main_ptr = NULL;
    
    /* Initialize based on runtime condition */
    if (condition & 1) {
        volatile_main_ptr = (generic_func_ptr)__hidden_builtin_1;
    }
#ifdef __i386__
    else if (condition & 2) {
        volatile_main_ptr = (generic_func_ptr)__builtin_ia32_rdtsc;
        /* Force reference to volatile_fptr */
        if (volatile_fptr) {
            result += (int)(long)volatile_fptr;
        }
    }
#endif
#ifdef __x86_64__
    else if (condition & 4) {
        volatile_main_ptr = (generic_func_ptr)__builtin_ia32_rdtsc64;
        if (volatile_rdtsc_ptr) {
            result += (int)volatile_rdtsc_ptr();
        }
    }
#endif
    
    /* Loop through function pointer array */
    for (int i = 0; func_array[i] != NULL; i++) {
        /* Opaque operation that can't be optimized away */
        result += (int)(long)func_array[i];
        
        /* Conditional that depends on runtime value */
        if ((condition >> i) & 1) {
            volatile_main_ptr = func_array[i];
        }
    }
    
    /* Final conditional that uses the volatile pointer */
    if (volatile_main_ptr && (condition & 0x80)) {
        /* This creates a reference that forces declaration processing */
        result += (int)(long)volatile_main_ptr;
    }
    
    /* Additional architecture-specific references */
#if defined(__i386__) || defined(__x86_64__)
    /* Reference x86 built-ins */
    result += (int)(long)__builtin_ia32_sfence;
#ifdef __x86_64__
    result += (int)(long)__builtin_ia32_mfence;
#endif
#endif
    
#if defined(__arm__) || defined(__aarch64__)
    /* Reference ARM built-ins */
#ifdef __arm__
    result += (int)(long)__builtin_arm_rbit;
#endif
#ifdef __aarch64__
    result += (int)(long)__builtin_aarch64_rbitll;
#endif
#endif
    
    return result & 0xFF; /* Return non-zero to ensure execution */
}

/* ============================================
   DUMMY IMPLEMENTATIONS TO SATISFY LINKER
   ============================================ */

/* These will be overridden by built-in implementations,
   but we provide them to avoid linker errors */

int __hidden_builtin_1(int x) {
    return x + 1;
}

int __hidden_builtin_2(float a, float b) {
    return (int)(a + b);
}

void __hidden_builtin_3(void* p) {
    *(volatile int*)p = 0;
}

volatile int* __hidden_builtin_4(volatile int* p, int x) {
    *p = x;
    return p;
}
