/* Built-in function visibility test program */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global volatile variable to prevent optimization */
volatile int global_seed = 0;

/* ============================================
   DECLARATION OF PROTOTYPES WITH TARGET ATTRIBUTES
   ============================================ */

/* Prototype 1: Full attribute combination */
extern int __attribute__((visibility("hidden"), used, artificial, noinline))
__hidden_builtin_1(int x) __asm__("__hidden_builtin_1");

/* Prototype 2: Different attribute order */
int __attribute__((externally_visible, visibility("hidden"), artificial))
__hidden_builtin_2(void) __asm__("__hidden_builtin_2");

/* Prototype 3: With no_return */
extern void __attribute__((visibility("hidden"), noreturn, used, artificial))
__hidden_builtin_3(int *ptr);

/* Prototype 4: With const attribute */
extern int __attribute__((visibility("hidden"), const, artificial))
__hidden_builtin_4(int a, int b) __asm__("__hidden_builtin_4");

/* ============================================
   TARGET-SPECIFIC BUILT-IN DECLARATIONS
   ============================================ */

/* x86/x86_64 specific built-ins */
#if defined(__i386__) || defined(__x86_64__) || defined(__amd64__)

/* Use actual GCC x86 built-ins */
extern long long __attribute__((visibility("hidden"), used, artificial))
__builtin_ia32_rdtsc(void);

extern void __attribute__((visibility("hidden"), used, artificial))
__builtin_ia32_clflush(const void *);

extern unsigned int __attribute__((visibility("hidden"), used, artificial))
__builtin_ia32_crc32qi(unsigned int, unsigned char);

extern int __attribute__((visibility("hidden"), used, artificial))
__builtin_ia32_addcarryx_u32(unsigned char, unsigned int, unsigned int, unsigned int *);

/* Define function pointers for x86 built-ins */
typedef long long (*rdtsc_ptr_t)(void);
typedef void (*clflush_ptr_t)(const void *);
typedef unsigned int (*crc32_ptr_t)(unsigned int, unsigned char);

#elif defined(__arm__) || defined(__aarch64__)

/* ARM specific built-ins */
extern unsigned int __attribute__((visibility("hidden"), used, artificial))
__builtin_arm_rbit(unsigned int);

extern void __attribute__((visibility("hidden"), used, artificial))
__builtin_arm_dmb(unsigned int);

extern int __attribute__((visibility("hidden"), used, artificial))
__builtin_arm_clz(int);

/* Define function pointers for ARM built-ins */
typedef unsigned int (*rbit_ptr_t)(unsigned int);
typedef void (*dmb_ptr_t)(unsigned int);
typedef int (*clz_ptr_t)(int);

#elif defined(__powerpc__) || defined(__ppc__) || defined(__PPC__)

/* PowerPC specific built-ins */
extern int __attribute__((visibility("hidden"), used, artificial))
__builtin_ppc_popcntb(int);

extern double __attribute__((visibility("hidden"), used, artificial))
__builtin_ppc_frsqrte(double);

/* Define function pointers for PowerPC built-ins */
typedef int (*popcntb_ptr_t)(int);
typedef double (*frsqrte_ptr_t)(double);

#else
/* Generic fallback - declare as weak symbols */
extern int __attribute__((visibility("hidden"), weak, used, artificial))
__generic_hidden_builtin(int x);

extern void __attribute__((visibility("hidden"), weak, used, artificial))
__generic_hidden_void(void);
#endif

/* ============================================
   VOLATILE FUNCTION POINTERS
   ============================================ */

/* Array of volatile function pointers to prevent optimization */
typedef int (*func_ptr_t)(int);
volatile func_ptr_t volatile_funcs[4];

/* ============================================
   HELPER FUNCTIONS
   ============================================ */

/* Opaque function to confuse optimizer */
static int __attribute__((noinline)) get_opaque_value(void) {
    return global_seed ^ 0xDEADBEEF;
}

/* Function that uses built-in addresses in non-optimizable way */
static void __attribute__((noinline, optimize("O0"))) 
use_builtin_addresses(int argc, char **argv) {
    volatile int result = 0;
    
    /* Initialize volatile function pointers */
    volatile_funcs[0] = (func_ptr_t)__hidden_builtin_1;
    volatile_funcs[1] = (func_ptr_t)__hidden_builtin_2;
    volatile_funcs[2] = (func_ptr_t)__hidden_builtin_4;
    
    /* Use argv to create unpredictable condition */
    int selector = (argc > 1) ? argv[1][0] : 'A';
    
    /* Non-optimizable comparison chain */
    for (int i = 0; i < 3; i++) {
        if ((selector & (1 << i)) && volatile_funcs[i] != NULL) {
            /* Force compiler to consider the function address */
            result += (long)volatile_funcs[i];
        }
    }
    
    /* Store result in global to prevent dead code elimination */
    global_seed = result;
}

/* ============================================
   TARGET-SPECIFIC BUILT-IN USAGE
   ============================================ */

static void __attribute__((noinline)) use_target_builtins(void) {
#if defined(__i386__) || defined(__x86_64__) || defined(__amd64__)
    volatile rdtsc_ptr_t rdtsc_ptr = __builtin_ia32_rdtsc;
    volatile clflush_ptr_t clflush_ptr = __builtin_ia32_clflush;
    
    /* Create opaque condition */
    if (get_opaque_value() > 0) {
        /* Reference the built-ins */
        long long ts = rdtsc_ptr();
        global_seed = (int)(ts & 0xFFFFFFFF);
    }
    
#elif defined(__arm__) || defined(__aarch64__)
    volatile rbit_ptr_t rbit_ptr = __builtin_arm_rbit;
    volatile clz_ptr_t clz_ptr = __builtin_arm_clz;
    
    if (get_opaque_value() != 0) {
        unsigned int val = rbit_ptr(0x12345678);
        global_seed = clz_ptr(val);
    }
    
#elif defined(__powerpc__) || defined(__ppc__) || defined(__PPC__)
    volatile popcntb_ptr_t popcntb_ptr = __builtin_ppc_popcntb;
    
    if (get_opaque_value() < 100) {
        int cnt = popcntb_ptr(0xFF);
        global_seed = cnt;
    }
#endif
}

/* ============================================
   MAIN FUNCTION
   ============================================ */

int main(int argc, char **argv) {
    /* Initialize with unpredictable value */
    global_seed = (argc > 0) ? argc : 1;
    
    /* Force processing of built-in declarations */
    use_builtin_addresses(argc, argv);
    
    /* Use target-specific built-ins */
    use_target_builtins();
    
    /* Additional opaque operations with function pointers */
    volatile int dummy = 0;
    for (int i = 0; i < 4; i++) {
        if (volatile_funcs[i] != NULL) {
            dummy += (long)volatile_funcs[i];
        }
    }
    
    /* Prevent dead code elimination */
    if (dummy != 0) {
        printf("Result: %d (seed: %d)\n", dummy, global_seed);
    }
    
    return global_seed & 1;
}

/* ============================================
   DUMMY IMPLEMENTATIONS (to satisfy linker if needed)
   ============================================ */

/* These might be needed if the built-ins aren't resolved */
int __hidden_builtin_1(int x) { return x + global_seed; }
int __hidden_builtin_2(void) { return global_seed ^ 0xCAFEBABE; }
void __hidden_builtin_3(int *ptr) { if (ptr) *ptr = global_seed; while(1); }
int __hidden_builtin_4(int a, int b) { return a * b + global_seed; }
