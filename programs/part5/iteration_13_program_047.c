/* Built-in function declaration stress test for GCC coverage */
#include <stdio.h>
#include <stdlib.h>

/* Prevent compiler from optimizing away our calls */
volatile int global_counter = 0;
volatile int use_builtin_trap = 0;
volatile int use_builtin_unreachable = 0;
volatile int use_target_specific = 0;

/* Helper function to create additional declaration contexts */
static void __attribute__((noinline)) 
use_builtins_inner(volatile int selector) {
    /* Reference built-ins without declaration */
    if (selector & 1) {
        /* Force reference to __builtin_expect without prototype */
        int (*expect_ptr)(long, long) = (int (*)(long, long))__builtin_expect;
        if (expect_ptr) global_counter++;
    }
    
    if (selector & 2) {
        /* Force reference to __builtin_constant_p without prototype */
        int (*const_p_ptr)(int) = (int (*)(int))__builtin_constant_p;
        if (const_p_ptr) global_counter += 2;
    }
}

/* Another helper with architecture-specific built-ins */
static void __attribute__((noinline))
use_arch_builtins(void) {
#ifdef __x86_64__
    /* x86 specific built-in without prototype */
    unsigned long long (*rdtsc_ptr)(void) = 
        (unsigned long long (*)(void))__builtin_ia32_rdtsc;
    if (rdtsc_ptr) global_counter += 3;
    
    /* SSE built-in */
    void (*sfence_ptr)(void) = (void (*)(void))__builtin_ia32_sfence;
    if (sfence_ptr) global_counter += 5;
#endif

#ifdef __arm__
    /* ARM specific built-in without prototype */
    unsigned int (*rbit_ptr)(unsigned int) = 
        (unsigned int (*)(unsigned int))__builtin_arm_rbit;
    if (rbit_ptr) global_counter += 7;
#endif

#ifdef __aarch64__
    /* AArch64 specific built-in */
    unsigned long (*clz_ptr)(unsigned long) = 
        (unsigned long (*)(unsigned long))__builtin_aarch64_clz_si;
    if (clz_ptr) global_counter += 11;
#endif
}

/* Function that takes address of undeclared built-ins */
static void __attribute__((noinline))
take_builtin_addresses(void) {
    /* Create function pointers to various built-ins */
    void (*trap_ptr)(void) = (void (*)(void))__builtin_trap;
    void (*unreachable_ptr)(void) = (void (*)(void))__builtin_unreachable;
    
    /* Use inline assembly to reference built-in names */
#ifdef __GNUC__
    /* This creates additional references to the symbols */
    asm volatile("" : : "i"(__builtin_trap), "i"(__builtin_unreachable));
    
#ifdef __x86_64__
    asm volatile("" : : "i"(__builtin_ia32_rdtsc));
#endif
    
#ifdef __arm__
    asm volatile("" : : "i"(__builtin_arm_rbit));
#endif
#endif
    
    /* Store pointers to prevent optimization */
    volatile void* volatile_ptr = trap_ptr;
    if (volatile_ptr) global_counter += 13;
    
    volatile_ptr = unreachable_ptr;
    if (volatile_ptr) global_counter += 17;
}

int main(void) {
    int i;
    volatile int checksum = 0;
    
    printf("Starting built-in declaration stress test...\n");
    
    /* Loop with complex control flow around built-in calls */
    for (i = 0; i < 100; i++) {
        /* Volatile conditions to prevent optimization */
        use_builtin_trap = (i % 7 == 0);
        use_builtin_unreachable = (i % 11 == 0);
        use_target_specific = (i % 13 == 0);
        
        /* Path that calls __builtin_trap without declaration */
        if (use_builtin_trap) {
            /* Direct call to undeclared built-in */
            __builtin_trap();
            checksum += 1;
        }
        
        /* Path that calls __builtin_unreachable without declaration */
        if (use_builtin_unreachable) {
            /* Direct call to undeclared built-in */
            __builtin_unreachable();
            checksum += 2;
        }
        
        /* Path using target-specific built-ins */
        if (use_target_specific) {
#ifdef __x86_64__
            /* Call x86 built-in without prototype */
            unsigned long long result = __builtin_ia32_rdtsc();
            checksum += (result & 0xFF);
#endif
            
#ifdef __arm__
            /* Call ARM built-in without prototype */
            unsigned int result = __builtin_arm_rbit(0x12345678);
            checksum += (result & 0xFF);
#endif
        }
        
        /* Call helper functions that reference built-ins */
        use_builtins_inner(i);
        
        if (i % 17 == 0) {
            use_arch_builtins();
        }
        
        if (i % 19 == 0) {
            take_builtin_addresses();
        }
        
        /* Reference other common built-ins without prototypes */
        if (i % 23 == 0) {
            /* __builtin_popcount without declaration */
            int (*popcount_ptr)(unsigned int) = 
                (int (*)(unsigned int))__builtin_popcount;
            if (popcount_ptr) checksum += popcount_ptr(i);
        }
        
        if (i % 29 == 0) {
            /* __builtin_bswap32 without declaration */
            unsigned int (*bswap32_ptr)(unsigned int) = 
                (unsigned int (*)(unsigned int))__builtin_bswap32;
            if (bswap32_ptr) checksum += bswap32_ptr(i) & 0xFF;
        }
        
        /* Prevent infinite loops from unreachable/trap */
        if (i > 50) {
            use_builtin_trap = 0;
            use_builtin_unreachable = 0;
        }
    }
    
    /* Use the global counter to prevent dead code elimination */
    checksum += global_counter;
    
    printf("Checksum: %d\n", checksum);
    printf("Global counter: %d\n", global_counter);
    
    /* Final reference to built-ins in return path */
    if (checksum > 1000) {
        /* Take address one more time */
        void (*return_trap)(void) = (void (*)(void))__builtin_trap;
        if (return_trap) {
            printf("Built-in trap function address obtained\n");
        }
    }
    
    return checksum & 0xFF;
}

/* Additional global references to built-ins */
static void* __attribute__((used)) 
global_builtin_refs[] = {
    (void*)__builtin_trap,
    (void*)__builtin_unreachable,
    (void*)__builtin_expect,
    (void*)__builtin_constant_p,
#ifdef __x86_64__
    (void*)__builtin_ia32_rdtsc,
    (void*)__builtin_ia32_sfence,
#endif
#ifdef __arm__
    (void*)__builtin_arm_rbit,
#endif
    (void*)__builtin_popcount,
    (void*)__builtin_bswap32,
    NULL
};
