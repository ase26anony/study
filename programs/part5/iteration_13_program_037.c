/* Built-in function reference test to trigger default_builtin_extdecl hook */
#include <stdio.h>
#include <stdlib.h>

/* Prevent optimization of variables */
static volatile int global_counter = 0;
static volatile int use_builtin = 1;

/* Helper function to create additional scope for built-in references */
__attribute__((noinline))
static void use_builtin_functions(int selector) {
    volatile static int local_state = 0;
    
    /* Reference built-ins without prototypes */
    if (selector & 1) {
        /* Generic built-in without declaration */
        int result = __builtin_expect(global_counter > 100, 0);
        local_state += result;
    }
    
    if (selector & 2) {
        /* Another generic built-in */
        int is_const = __builtin_constant_p(selector);
        local_state += is_const;
    }
    
#ifdef __GNUC__
    /* Architecture-specific built-ins */
    #ifdef __x86_64__
    /* x86 specific built-in - will need external declaration */
    unsigned long long (*rdtsc_ptr)(void) = (unsigned long long (*)(void))__builtin_ia32_rdtsc;
    if (rdtsc_ptr && (selector & 4)) {
        unsigned long long cycles = rdtsc_ptr();
        local_state += (cycles & 1);
    }
    #endif
    
    #ifdef __arm__
    /* ARM specific built-in */
    unsigned int (*rbit_ptr)(unsigned int) = (unsigned int (*)(unsigned int))__builtin_arm_rbit;
    if (rbit_ptr && (selector & 8)) {
        unsigned int val = rbit_ptr(0x12345678);
        local_state += (val & 1);
    }
    #endif
    
    #ifdef __aarch64__
    /* AArch64 specific built-in */
    unsigned long (*clz_ptr)(unsigned long) = (unsigned long (*)(unsigned long))__builtin_aarch64_clz;
    if (clz_ptr && (selector & 16)) {
        unsigned long val = clz_ptr(0xFFFFFFFF);
        local_state += (val & 1);
    }
    #endif
#endif
    
    global_counter += local_state;
}

/* Another helper with different built-in usage pattern */
__attribute__((noinline, noclone))
static void builtin_trap_test(int condition) {
    volatile int should_trap = condition;
    
    /* Use inline assembly to reference built-in name */
    #ifdef __x86_64__
    asm volatile("" : : "i"(__builtin_ia32_rdtsc) : "memory");
    #endif
    
    if (should_trap > 1000) {
        /* Reference __builtin_trap without declaration */
        void (*trap_ptr)(void) = (void (*)(void))__builtin_trap;
        if (trap_ptr) {
            trap_ptr();
        }
    } else if (should_trap < 0) {
        /* Reference __builtin_unreachable without declaration */
        void (*unreachable_ptr)(void) = (void (*)(void))__builtin_unreachable;
        if (unreachable_ptr) {
            unreachable_ptr();
        }
    }
}

/* Function that takes address of various built-ins */
__attribute__((noinline))
static void take_builtin_addresses(void) {
    /* Take addresses of multiple built-ins without declarations */
    void *builtin_ptrs[10] = {0};
    int idx = 0;
    
    /* Generic built-ins */
    builtin_ptrs[idx++] = (void *)__builtin_expect;
    builtin_ptrs[idx++] = (void *)__builtin_constant_p;
    builtin_ptrs[idx++] = (void *)__builtin_trap;
    builtin_ptrs[idx++] = (void *)__builtin_unreachable;
    
#ifdef __GNUC__
    #ifdef __x86_64__
    builtin_ptrs[idx++] = (void *)__builtin_ia32_rdtsc;
    builtin_ptrs[idx++] = (void *)__builtin_ia32_pause;
    #endif
    
    #ifdef __arm__
    builtin_ptrs[idx++] = (void *)__builtin_arm_rbit;
    #endif
    
    /* More generic GCC built-ins */
    builtin_ptrs[idx++] = (void *)__builtin_popcount;
    builtin_ptrs[idx++] = (void *)__builtin_ffs;
    builtin_ptrs[idx++] = (void *)__builtin_clz;
#endif
    
    /* Use the pointers to prevent optimization */
    volatile int sum = 0;
    for (int i = 0; i < idx; i++) {
        sum += ((unsigned long)builtin_ptrs[i] & 1);
    }
    global_counter += sum;
}

int main(void) {
    volatile int iteration = 0;
    volatile int checksum = 0;
    
    printf("Starting built-in reference test...\n");
    
    /* Loop with complex control flow */
    for (iteration = 0; iteration < 100; iteration++) {
        volatile int selector = (iteration * 1103515245 + 12345) & 0x7FFFFFFF;
        
        /* Call helper with built-in references */
        use_builtin_functions(selector);
        
        /* Every 10 iterations, test trap conditions */
        if ((iteration % 10) == 0) {
            builtin_trap_test(selector);
        }
        
        /* Every 25 iterations, take addresses of built-ins */
        if ((iteration % 25) == 0) {
            take_builtin_addresses();
        }
        
        /* Mix in direct built-in calls without prototypes */
        if (selector & 0x100) {
            /* These calls have no prior declaration */
            int predicted = __builtin_expect((selector & 0xFF) > 128, 1);
            checksum += predicted;
        }
        
        if (selector & 0x200) {
            int is_const = __builtin_constant_p(iteration);
            checksum += is_const;
        }
        
        /* Use architecture-specific built-ins directly */
        #ifdef __x86_64__
        if (selector & 0x400) {
            /* Direct call without prototype */
            unsigned long long cycles = __builtin_ia32_rdtsc();
            checksum += (cycles & 0xFF);
        }
        #endif
        
        /* Prevent infinite loops with __builtin_unreachable */
        if (selector == 0xDEADBEEF) {
            __builtin_unreachable();
        }
        
        global_counter++;
    }
    
    /* Final checksum computation */
    checksum += global_counter;
    checksum ^= iteration;
    
    /* Use __builtin_constant_p one more time */
    int final_const_check = __builtin_constant_p(checksum);
    
    printf("Test completed. Checksum: %d (constant: %d)\n", 
           checksum, final_const_check);
    
    /* Use __builtin_trap on impossible condition */
    if (checksum == 0xDEADBEEF) {
        __builtin_trap();
    }
    
    return checksum & 0xFF;
}

/* Additional global references to built-ins */
#ifdef __GNUC__
/* These declarations are intentionally omitted to force 
   default_builtin_extdecl to create them */
/* No prototype for __builtin_cpu_supports */
/* No prototype for __builtin_cpu_init */

/* Global function pointer table */
static void * volatile builtin_refs[] = {
    (void *)__builtin_cpu_supports,
    (void *)__builtin_cpu_init,
    (void *)__builtin_expect,
    (void *)__builtin_constant_p,
#ifdef __x86_64__
    (void *)__builtin_ia32_rdtsc,
#endif
    0
};
#endif
