/* Built-in function declaration stress test to trigger default_builtin_extdecl */
#include <stdio.h>
#include <stdlib.h>

/* Prevent compiler from optimizing away our volatile variables */
static volatile int global_counter = 0;
static volatile int use_builtin = 1;

/* Helper function to create additional scope for built-in references */
__attribute__((noinline))
static void use_builtin_functions(int selector) {
    volatile static int local_state = 0;
    
    /* Reference various built-ins without declaration */
    if (selector & 1) {
        /* Force reference to common built-ins */
        if (local_state == 0) {
            /* __builtin_expect without prototype */
            int result = __builtin_expect(selector > 0, 1);
            local_state = result;
        }
        
        /* __builtin_constant_p without prototype */
        if (__builtin_constant_p(selector)) {
            local_state = 2;
        }
    }
    
    if (selector & 2) {
        /* __builtin_unreachable reference */
        if (local_state < 0) {
            __builtin_unreachable();
        }
    }
    
    /* Function pointer to built-in without declaration */
    void (*builtin_ptr)(void) = (void (*)(void))__builtin_trap;
    
    /* Store pointer in volatile to prevent optimization */
    volatile void *volatile_ptr = (void *)builtin_ptr;
    
    /* Use inline assembly to reference built-in name */
    asm volatile("" : : "i"(__builtin_trap) : "memory");
}

/* Another helper with architecture-specific built-ins */
__attribute__((noinline))
static void use_arch_builtins(void) {
    volatile long long timestamp = 0;
    
#ifdef __x86_64__
    /* x86 specific built-in without prototype */
    timestamp = __builtin_ia32_rdtsc();
    
    /* Take address of x86 built-in */
    unsigned long long (*rdtsc_ptr)(void) = 
        (unsigned long long (*)(void))__builtin_ia32_rdtsc;
    
    /* Use in inline assembly */
    asm volatile("" : : "i"(__builtin_ia32_rdtsc) : "memory");
    
    /* Call through pointer */
    if (global_counter > 1000) {
        timestamp = rdtsc_ptr();
    }
#endif

#ifdef __arm__
    /* ARM specific built-in without prototype */
    unsigned int reversed = __builtin_arm_rbit(global_counter);
    
    /* Take address of ARM built-in */
    unsigned int (*rbit_ptr)(unsigned int) = 
        (unsigned int (*)(unsigned int))__builtin_arm_rbit;
    
    asm volatile("" : : "i"(__builtin_arm_rbit) : "memory");
    
    if (global_counter > 1000) {
        reversed = rbit_ptr(global_counter);
    }
#endif

#ifdef __aarch64__
    /* AArch64 specific built-in */
    unsigned long long aarch64_val = __builtin_aarch64_ld64();
    
    unsigned long long (*ld64_ptr)(void) = 
        (unsigned long long (*)(void))__builtin_aarch64_ld64;
    
    asm volatile("" : : "i"(__builtin_aarch64_ld64) : "memory");
#endif

    /* Reference generic built-ins */
    if (timestamp > 1000000) {
        /* __builtin_popcount without prototype */
        int bits = __builtin_popcountll(timestamp);
        global_counter += bits;
    }
}

/* Main function with complex control flow */
int main(void) {
    volatile int i, j;
    int checksum = 0;
    
    /* Complex loop with multiple built-in references */
    for (i = 0; i < 100; i++) {
        volatile int path_selector = global_counter % 8;
        
        /* Different paths call different undeclared built-ins */
        switch (path_selector) {
            case 0:
                /* __builtin_trap without prototype */
                if (global_counter < 0) {
                    __builtin_trap();
                }
                break;
                
            case 1:
                /* __builtin_unreachable without prototype */
                if (global_counter > 1000000) {
                    __builtin_unreachable();
                }
                break;
                
            case 2:
                /* __builtin_expect without prototype */
                if (__builtin_expect(global_counter > 50, 0)) {
                    checksum += 1;
                }
                break;
                
            case 3:
                /* __builtin_ffs without prototype */
                checksum += __builtin_ffs(global_counter);
                break;
                
            case 4:
                /* __builtin_clz without prototype */
                if (global_counter != 0) {
                    checksum += __builtin_clz(global_counter);
                }
                break;
                
            case 5:
                /* __builtin_ctz without prototype */
                if (global_counter != 0) {
                    checksum += __builtin_ctz(global_counter);
                }
                break;
                
            case 6:
                /* __builtin_parity without prototype */
                checksum += __builtin_parity(global_counter);
                break;
                
            case 7:
                /* __builtin_bswap32 without prototype */
                checksum += __builtin_bswap32(global_counter);
                break;
        }
        
        /* Take address of various built-ins */
        void (*trap_ptr)(void) = (void (*)(void))__builtin_trap;
        int (*clz_ptr)(int) = (int (*)(int))__builtin_clz;
        int (*expect_ptr)(long, int) = (int (*)(long, int))__builtin_expect;
        
        /* Store pointers in volatile variables */
        volatile void *volatile_trap_ptr = (void *)trap_ptr;
        volatile void *volatile_clz_ptr = (void *)clz_ptr;
        volatile void *volatile_expect_ptr = (void *)expect_ptr;
        
        /* Call helper functions */
        use_builtin_functions(global_counter);
        use_arch_builtins();
        
        /* Use inline assembly with built-in references */
        asm volatile("" 
                     : 
                     : "i"(__builtin_trap), 
                       "i"(__builtin_clz), 
                       "i"(__builtin_expect)
                     : "memory");
        
        global_counter++;
    }
    
    /* Final built-in references */
    if (checksum < 0) {
        __builtin_trap();
    }
    
    /* Reference CPU built-ins without prototype */
#ifdef __GNUC__
    /* __builtin_cpu_supports without prototype */
    if (__builtin_cpu_supports("sse2")) {
        checksum += 1;
    }
    
    /* __builtin_cpu_init without prototype */
    __builtin_cpu_init();
#endif
    
    printf("Checksum: %d\n", checksum);
    printf("Global counter: %d\n", global_counter);
    
    return checksum == 0 ? 0 : 1;
}

/* Additional function to create more references */
__attribute__((constructor))
static void init_builtin_refs(void) {
    /* Reference more built-ins in constructor */
    volatile int init_val = __builtin_constant_p(42) ? 1 : 0;
    
    /* __builtin_return_address without prototype */
    void *ret_addr = __builtin_return_address(0);
    
    /* __builtin_frame_address without prototype */
    void *frame_addr = __builtin_frame_address(0);
    
    /* Use addresses to prevent optimization */
    global_counter += (long)ret_addr % 256;
    global_counter += (long)frame_addr % 256;
}
