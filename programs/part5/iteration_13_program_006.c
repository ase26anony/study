/* Built-in function declaration pressure test
 * Designed to trigger default_builtin_extdecl in targhooks.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent compiler from optimizing away our calls */
volatile int global_counter = 0;
volatile int use_trap = 0;
volatile int use_unreachable = 1;

/* Helper function to create additional declaration contexts */
static void __attribute__((noinline)) 
pressure_builtin_declaration(volatile int selector) {
    /* Reference built-ins without declaration */
    if (selector & 1) {
        /* Force reference to common built-ins */
        int result = __builtin_expect(global_counter > 100, 0);
        (void)result;
        
        /* Check for constant - another built-in */
        if (__builtin_constant_p(selector)) {
            global_counter++;
        }
    }
    
    /* Architecture-specific built-ins */
#ifdef __x86_64__
    /* x86 specific built-in without declaration */
    if (selector & 2) {
        unsigned long long tsc;
        /* Reference without prototype */
        tsc = __builtin_ia32_rdtsc();
        global_counter += (int)(tsc & 0xFF);
    }
#endif

#ifdef __arm__
    /* ARM specific built-in without declaration */
    if (selector & 4) {
        unsigned int val = 0x12345678;
        unsigned int reversed;
        /* Reference without prototype */
        reversed = __builtin_arm_rbit(val);
        global_counter += (reversed & 0xFF);
    }
#endif

#ifdef __aarch64__
    /* ARM64 specific built-in */
    if (selector & 8) {
        unsigned long long val = 0x123456789ABCDEF0ULL;
        unsigned long long reversed;
        /* Reference without prototype */
        reversed = __builtin_aarch64_rbitdi(val);
        global_counter += (int)(reversed & 0xFF);
    }
#endif
}

/* Another helper with function pointer manipulation */
static void __attribute__((noinline))
builtin_function_pointers(void) {
    /* Create function pointers to undeclared built-ins */
    void (*trap_ptr)(void) = (void (*)(void))__builtin_trap;
    void (*unreachable_ptr)(void) = (void (*)(void))__builtin_unreachable;
    
    /* Store in volatile to prevent optimization */
    volatile void* volatile_trap_ptr = (void*)trap_ptr;
    volatile void* volatile_unreachable_ptr = (void*)unreachable_ptr;
    
    /* Use inline assembly to reference the symbols */
#ifdef __GNUC__
    /* Reference built-in names in assembly constraints */
    asm volatile("" : : "i"(__builtin_trap), "i"(__builtin_unreachable));
    
    /* More assembly references for target-specific built-ins */
#ifdef __x86_64__
    asm volatile("" : : "i"(__builtin_ia32_rdtsc));
#endif
#ifdef __arm__
    asm volatile("" : : "i"(__builtin_arm_rbit));
#endif
#endif
    
    (void)volatile_trap_ptr;
    (void)volatile_unreachable_ptr;
}

/* Main test function */
int main(void) {
    int i, checksum = 0;
    
    printf("Starting built-in declaration pressure test...\n");
    
    /* Loop with complex control flow */
    for (i = 0; i < 100; i++) {
        volatile int path_selector = i & 0xF;
        
        /* Call helper that references undeclared built-ins */
        pressure_builtin_declaration(path_selector);
        
        /* Direct calls to undeclared built-ins in different paths */
        if (use_trap && (i % 13 == 0)) {
            /* This should trigger declaration generation */
            __builtin_trap();
        }
        
        if (use_unreachable && (i % 17 == 0)) {
            /* Another built-in without declaration */
            if (global_counter > 1000) {
                __builtin_unreachable();
            }
        }
        
        /* More built-in references */
        if (i % 5 == 0) {
            /* __builtin_constant_p without declaration */
            int is_const = __builtin_constant_p(i);
            checksum += is_const;
        }
        
        if (i % 7 == 0) {
            /* __builtin_expect without declaration */
            long likely = __builtin_expect(global_counter < 50, 1);
            checksum += (int)likely;
        }
        
        /* Update volatile state to affect control flow */
        global_counter += (i & 0x3F);
        
        /* Periodically manipulate function pointers */
        if (i % 19 == 0) {
            builtin_function_pointers();
        }
    }
    
    /* Final built-in references */
#ifdef __GNUC__
    /* Use __builtin_popcount without declaration */
    int popcnt = __builtin_popcount(checksum);
    printf("Checksum: %d, Population count: %d\n", checksum, popcnt);
    
    /* __builtin_ffs without declaration */
    int ffs_result = __builtin_ffs(checksum | 1);
    printf("First set bit: %d\n", ffs_result);
#endif
    
    /* Reference more built-ins in final calculation */
    int final_value = checksum;
    
    /* __builtin_abs without declaration */
    final_value = __builtin_abs(final_value);
    
    /* __builtin_clz without declaration (if available) */
    if (final_value != 0) {
        unsigned int leading_zeros = __builtin_clz((unsigned int)final_value);
        printf("Leading zeros: %u\n", leading_zeros);
    }
    
    printf("Test completed. Global counter: %d\n", global_counter);
    
    return final_value & 0xFF;
}

/* Additional global scope references to built-ins */
#ifdef __GNUC__
/* Force external references at global scope */
static volatile long (*volatile_rdtsc_ptr)(void) = 
#ifdef __x86_64__
    (long (*)(void))__builtin_ia32_rdtsc;
#else
    (long (*)(void))0;
#endif

/* Array of function pointers to various built-ins */
static void* builtin_refs[] = {
    (void*)__builtin_trap,
    (void*)__builtin_unreachable,
    (void*)__builtin_expect,
    (void*)__builtin_constant_p,
#ifdef __x86_64__
    (void*)__builtin_ia32_rdtsc,
#endif
#ifdef __arm__
    (void*)__builtin_arm_rbit,
#endif
    0
};
#endif
