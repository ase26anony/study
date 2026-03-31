/* Built-in function declaration stress test for GCC coverage */
#include <stdio.h>
#include <stdlib.h>

/* Prevent optimization of critical variables */
static volatile int state = 0;
static volatile long counter = 0;

/* Helper function to create additional declaration contexts */
__attribute__((noinline)) 
static void use_builtin_variants(int selector) {
    volatile int local_state = selector;
    
    /* Reference various built-ins without declaration */
    if (local_state & 1) {
        /* Common built-in without prototype */
        int result = __builtin_expect(local_state > 0, 1);
        (void)result;
    }
    
    if (local_state & 2) {
        /* Another common built-in */
        int is_const = __builtin_constant_p(local_state);
        (void)is_const;
    }
    
#ifdef __x86_64__
    if (local_state & 4) {
        /* x86-specific built-in */
        unsigned long long tsc = __builtin_ia32_rdtsc();
        (void)tsc;
    }
#endif

#ifdef __arm__
    if (local_state & 4) {
        /* ARM-specific built-in */
        unsigned int reversed = __builtin_arm_rbit(local_state);
        (void)reversed;
    }
#endif

#ifdef __aarch64__
    if (local_state & 4) {
        /* AArch64-specific built-in */
        unsigned long long reversed = __builtin_aarch64_rbitll(local_state);
        (void)reversed;
    }
#endif
}

/* Function pointer manipulation for built-ins */
typedef void (*builtin_func_ptr)(void);
typedef unsigned long long (*tsc_func_ptr)(void);

int main(void) {
    volatile int i;
    volatile long checksum = 0;
    
    /* Initialize volatile state */
    state = 42;
    counter = 0;
    
    /* Loop with complex control flow */
    for (i = 0; i < 10; i++) {
        counter++;
        
        /* Different paths based on volatile state */
        if (state > 20) {
            /* Use trap built-in without declaration */
            if (counter % 3 == 0) {
                /* This should trigger declaration generation */
                __builtin_trap();
            }
        } else {
            /* Use unreachable built-in without declaration */
            if (counter % 5 == 0) {
                __builtin_unreachable();
            }
        }
        
        /* Take address of built-in functions */
#ifdef __x86_64__
        {
            /* Create function pointer to x86 built-in */
            tsc_func_ptr tsc_func = (tsc_func_ptr)__builtin_ia32_rdtsc;
            /* Call through pointer */
            if (tsc_func) {
                unsigned long long val = tsc_func();
                checksum ^= (val & 0xFFFF);
            }
        }
#endif
        
        /* Use inline assembly to reference built-in */
        asm volatile (
            "/* Reference builtin_expect */"
            : 
            : "i"(__builtin_expect)
            : "memory"
        );
        
        /* Call helper with different selectors */
        use_builtin_variants(i);
        
        /* More built-in references in different contexts */
        if (i % 2 == 0) {
            /* Use popcount built-in if available */
#ifdef __GNUC__
            int bits = __builtin_popcount(i);
            checksum += bits;
#endif
        }
        
        /* Change state to alter control flow */
        state ^= i;
    }
    
    /* Final built-in reference */
    int final_pred = __builtin_expect(checksum != 0, 1);
    
    /* Use assembly to create another reference */
    asm volatile (
        "/* Force reference to multiple builtins */"
        : 
        : "i"(__builtin_trap), "i"(__builtin_unreachable)
    );
    
    /* Print result to prevent elimination */
    printf("Checksum: %ld (pred: %d)\n", checksum, final_pred);
    
    /* One more path with built-in */
    if (checksum == 0) {
        /* This should never happen, but references builtin */
        __builtin_unreachable();
    }
    
    return final_pred ? 0 : 1;
}

/* Additional global scope references */
#ifdef __GNUC__
/* Global function pointer array */
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
