/* Built-in function declaration stress test to trigger default_builtin_extdecl */
#include <stdio.h>
#include <stdlib.h>

/* Prevent compiler from optimizing away our volatile variables */
static volatile int global_counter = 0;
static volatile void *volatile_func_ptr = NULL;

/* Helper function to create additional scope for built-in references */
__attribute__((noinline, cold))
static int use_builtins_internal(int selector) {
    volatile int local_result = 0;
    
    /* Take address of undeclared built-ins - forces declaration creation */
    void *builtin_addr;
    
#ifdef __x86_64__
    /* x86 specific built-in without prototype */
    builtin_addr = (void*)__builtin_ia32_rdtsc;
    local_result += (long)builtin_addr & 1;
#endif

#ifdef __arm__
    /* ARM specific built-in without prototype */
    builtin_addr = (void*)__builtin_arm_rbit;
    local_result += (long)builtin_addr & 1;
#endif

#ifdef __aarch64__
    /* ARM64 specific built-in */
    builtin_addr = (void*)__builtin_aarch64_get_fp;
    local_result += (long)builtin_addr & 1;
#endif

    /* Generic built-ins without prototypes */
    switch (selector & 3) {
        case 0:
            builtin_addr = (void*)__builtin_expect;
            break;
        case 1:
            builtin_addr = (void*)__builtin_constant_p;
            break;
        case 2:
            builtin_addr = (void*)__builtin_trap;
            break;
        case 3:
            builtin_addr = (void*)__builtin_unreachable;
            break;
    }
    
    /* Use inline assembly to reference built-in names */
    asm volatile("" : : "i"(__builtin_trap), "i"(__builtin_unreachable));
    
    return local_result + ((long)builtin_addr & 1);
}

/* Another helper with different built-in usage pattern */
__attribute__((noinline, cold))
static void call_through_pointer(void) {
    /* Create function pointer to undeclared built-in */
    void (*trap_func)(void) = __builtin_trap;
    void (*unreachable_func)(void) = __builtin_unreachable;
    
    /* Store in volatile pointer to prevent optimization */
    volatile_func_ptr = trap_func;
    
    /* Conditional call through pointer */
    if (global_counter & 0x100) {
        ((void (*)(void))volatile_func_ptr)();
    }
    
    /* Reference built-in in assembly constraint */
    asm volatile("" : : "r"(unreachable_func));
}

int main(void) {
    volatile int result = 0;
    volatile int i;
    
    printf("Starting built-in declaration stress test...\n");
    
    /* Loop with complex control flow around built-in calls */
    for (i = 0; i < 100; i++) {
        global_counter++;
        
        /* Different paths call different undeclared built-ins */
        if (global_counter & 1) {
            /* Call built-in without prototype */
            int r = __builtin_constant_p(i);
            result += r ? 1 : 0;
        }
        
        if (global_counter & 2) {
            /* Use __builtin_expect without prototype */
            long x = __builtin_expect(global_counter, 0);
            result += (int)x;
        }
        
        if (global_counter & 4) {
            /* Take address of target-specific built-in */
            void *addr;
#ifdef __x86_64__
            addr = (void*)__builtin_ia32_rdtsc;
#elif defined(__arm__)
            addr = (void*)__builtin_arm_rbit;
#else
            addr = (void*)__builtin_trap;
#endif
            result += ((long)addr >> 2) & 1;
        }
        
        if (global_counter & 8) {
            /* Call helper that uses built-ins internally */
            result += use_builtins_internal(global_counter);
        }
        
        if (global_counter & 16) {
            /* Potentially call __builtin_trap through pointer */
            call_through_pointer();
        }
        
        /* Mix in some __builtin_unreachable calls */
        if (global_counter > 1000) {
            __builtin_unreachable();
        }
    }
    
    /* Final forced reference to multiple built-ins */
    asm volatile("" : : 
        "i"(__builtin_ia32_rdtsc),
        "i"(__builtin_arm_rbit),
        "i"(__builtin_trap),
        "i"(__builtin_unreachable),
        "i"(__builtin_expect),
        "i"(__builtin_constant_p)
    );
    
    printf("Result: %d (counter: %d)\n", result, global_counter);
    
    /* One more conditional built-in call at the end */
    if (result & 1) {
        /* This should force declaration of __builtin_cpu_supports */
        int cpu_supports = __builtin_cpu_supports("sse2");
        printf("CPU supports SSE2: %d\n", cpu_supports);
    }
    
    return result & 0xFF;
}
