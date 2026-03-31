/* Built-in function stress test to trigger default_builtin_extdecl */
#include <stdio.h>
#include <stdlib.h>

/* Prevent compiler from optimizing away our calls */
static volatile int global_counter = 0;
static volatile int use_builtin_a = 1;
static volatile int use_builtin_b = 0;
static volatile int use_target_specific = 1;

/* Helper function to create additional declaration contexts */
__attribute__((noinline))
static void call_builtins_inner(int selector) {
    volatile static int inner_state = 0;
    
    /* Reference builtins without declaration */
    if (selector & 1) {
        /* Force reference to common builtins */
        if (inner_state++ > 100) {
            /* These calls have no prototype - should trigger extdecl */
            __builtin_trap();  /* No prototype */
        } else {
            __builtin_unreachable();  /* No prototype */
        }
    }
    
    /* Use __builtin_expect without prototype */
    if (__builtin_expect(selector > 50, 0)) {  /* No prototype */
        inner_state += 2;
    }
    
    /* Use __builtin_constant_p without prototype */
    if (!__builtin_constant_p(selector)) {  /* No prototype */
        inner_state *= 3;
    }
}

/* Another helper with architecture-specific builtins */
__attribute__((noinline))
static void call_target_builtins(void) {
    volatile long long timestamp = 0;
    
    /* Architecture-specific builtins without prototypes */
#ifdef __x86_64__
    /* x86 specific - no prototype */
    timestamp = __builtin_ia32_rdtsc();  /* No prototype */
    
    /* More x86 builtins */
    __builtin_ia32_mfence();  /* No prototype */
    __builtin_ia32_sfence();  /* No prototype */
#endif

#ifdef __arm__
    /* ARM specific - no prototype */
    timestamp = __builtin_arm_rbit(0x12345678);  /* No prototype */
#endif

#ifdef __aarch64__
    /* AArch64 specific */
    timestamp = __builtin_aarch64_rbit(0x12345678);  /* No prototype */
#endif

    /* Use the result to prevent optimization */
    global_counter += (int)(timestamp & 0xFF);
}

/* Function that takes addresses of builtins */
__attribute__((noinline))
static void take_builtin_addresses(void) {
    /* Create function pointers to undeclared builtins */
    void (*trap_ptr)(void);
    void (*unreachable_ptr)(void);
    long long (*rdtsc_ptr)(void);
    
    /* Take addresses without prototypes - should trigger extdecl */
    trap_ptr = __builtin_trap;  /* No prototype */
    unreachable_ptr = __builtin_unreachable;  /* No prototype */
    
#ifdef __x86_64__
    rdtsc_ptr = __builtin_ia32_rdtsc;  /* No prototype */
#endif

#ifdef __arm__
    rdtsc_ptr = (long long (*)(void))__builtin_arm_rbit;  /* No prototype */
#endif

    /* Store pointers in volatile to prevent optimization */
    volatile void* volatile_ptr = trap_ptr;
    (void)volatile_ptr;
}

/* Use inline assembly to reference builtin names */
static void asm_reference_builtins(void) {
    /* Reference builtin names in assembly constraints */
#ifdef __x86_64__
    asm volatile("" 
                 : /* no outputs */
                 : "i"(__builtin_ia32_rdtsc)  /* Reference as immediate */
                 : "memory");
#endif

    /* Reference generic builtins */
    asm volatile("/* %0 */" 
                 : /* no outputs */
                 : "i"(__builtin_trap)  /* Reference as immediate */
                 :);
}

int main(void) {
    int i, result = 0;
    
    printf("Starting built-in function stress test...\n");
    
    /* Main loop with multiple builtin references */
    for (i = 0; i < 100; i++) {
        volatile int choice = global_counter % 7;
        
        /* Different paths calling different undeclared builtins */
        switch (choice) {
            case 0:
                /* Direct call without prototype */
                __builtin_trap();  /* No prototype */
                break;
                
            case 1:
                /* Another direct call */
                __builtin_unreachable();  /* No prototype */
                break;
                
            case 2:
                /* Call through helper */
                call_builtins_inner(i);
                break;
                
            case 3:
                /* Target-specific builtins */
                call_target_builtins();
                break;
                
            case 4:
                /* Take addresses */
                take_builtin_addresses();
                break;
                
            case 5:
                /* Assembly references */
                asm_reference_builtins();
                break;
                
            case 6:
                /* Mixed usage */
                if (__builtin_expect(i > 50, 0)) {  /* No prototype */
                    __builtin_unreachable();  /* No prototype */
                }
                break;
        }
        
        /* Use __builtin_constant_p without prototype in loop */
        if (!__builtin_constant_p(i)) {  /* No prototype */
            global_counter++;
        }
        
        /* Occasionally use target-specific builtin */
        if (use_target_specific && (i % 13 == 0)) {
#ifdef __x86_64__
            volatile long long ts = __builtin_ia32_rdtsc();  /* No prototype */
            result += (int)(ts & 0xFF);
#endif
        }
    }
    
    /* Final computation using builtin addresses */
    void (*final_trap)(void) = __builtin_trap;  /* No prototype */
    void (*final_unreachable)(void) = __builtin_unreachable;  /* No prototype */
    
    /* Prevent optimization of function pointers */
    volatile int dummy = (long)final_trap | (long)final_unreachable;
    (void)dummy;
    
    /* Compute checksum */
    result += global_counter;
    printf("Result: %d\n", result);
    
    /* One last undeclared builtin call based on result */
    if (result > 1000) {
        __builtin_trap();  /* No prototype */
    } else {
        __builtin_unreachable();  /* No prototype */
    }
    
    return result & 0xFF;
}

/* Additional global references to builtins */
static void (*global_builtin_refs[])(void) = {
    __builtin_trap,      /* No prototype */
    __builtin_unreachable, /* No prototype */
    NULL
};

/* Another function in different compilation context */
__attribute__((constructor))
static void init_builtin_refs(void) {
    /* Reference builtins in constructor */
    volatile int x = 0;
    
    if (__builtin_constant_p(x)) {  /* No prototype */
        x = 1;
    }
    
    /* Use __builtin_expect */
    if (__builtin_expect(x == 0, 1)) {  /* No prototype */
        global_counter = 100;
    }
}
