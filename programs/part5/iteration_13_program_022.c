/* 
 * Built-in function declaration stress test
 * Designed to trigger default_builtin_extdecl in targhooks.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent compiler from optimizing away our calls */
volatile int global_counter = 0;
volatile int use_builtin_a = 1;
volatile int use_builtin_b = 0;
volatile int use_target_specific = 1;

/* Helper function to create additional declaration contexts */
__attribute__((noinline)) 
static void call_builtins_inner(volatile int selector) {
    /* Reference builtins without declaration */
    if (selector & 1) {
        /* Force compiler to create external decl for __builtin_trap */
        __builtin_trap();
    }
    
    if (selector & 2) {
        /* Force compiler to create external decl for __builtin_unreachable */
        __builtin_unreachable();
    }
    
    /* Use __builtin_expect without prototype */
    if (__builtin_expect(selector > 100, 0)) {
        global_counter++;
    }
    
    /* Use __builtin_constant_p without prototype */
    if (!__builtin_constant_p(selector)) {
        global_counter--;
    }
}

/* Another helper with architecture-specific builtins */
__attribute__((noinline))
static void call_target_builtins(void) {
#ifdef __x86_64__
    /* x86 specific builtin without declaration */
    unsigned long long tsc = __builtin_ia32_rdtsc();
    global_counter += (int)(tsc & 0xFF);
#endif

#ifdef __i386__
    /* 32-bit x86 variant */
    unsigned long long tsc = __builtin_ia32_rdtsc();
    global_counter += (int)(tsc & 0xFF);
#endif

#ifdef __arm__
    /* ARM specific builtin */
    unsigned int reversed = __builtin_arm_rbit(global_counter);
    global_counter = reversed & 0xFF;
#endif

#ifdef __aarch64__
    /* ARM64 specific builtin */
    unsigned long reversed = __builtin_aarch64_rbit(global_counter);
    global_counter = reversed & 0xFF;
#endif

#ifdef __powerpc__
    /* PowerPC specific builtin */
    int cntlzw = __builtin_ppc_cntlzw(global_counter);
    global_counter = cntlzw;
#endif
}

/* Function pointer test - taking address of undeclared builtins */
static void test_builtin_pointers(void) {
    /* Declare function pointers without knowing the prototype */
    void (*trap_ptr)(void);
    void (*unreachable_ptr)(void);
    
#ifdef __x86_64__
    unsigned long long (*rdtsc_ptr)(void);
#endif

    /* Take addresses - compiler must create declarations */
    trap_ptr = __builtin_trap;
    unreachable_ptr = __builtin_unreachable;
    
#ifdef __x86_64__
    rdtsc_ptr = __builtin_ia32_rdtsc;
#endif

    /* Call through pointers */
    if (global_counter > 1000) {
        trap_ptr();
    }
    
    if (global_counter < 0) {
        unreachable_ptr();
    }
    
#ifdef __x86_64__
    if (use_target_specific) {
        unsigned long long tsc = rdtsc_ptr();
        global_counter += (int)(tsc & 0x7F);
    }
#endif
}

/* Inline assembly references to builtins */
static void asm_references(void) {
    /* Reference builtins in inline asm to create additional uses */
    asm volatile(
        "/* Reference to builtin_trap */"
        :
        : "i"(__builtin_trap)
    );
    
    asm volatile(
        "/* Reference to builtin_unreachable */"
        :
        : "i"(__builtin_unreachable)
    );
    
#ifdef __x86_64__
    asm volatile(
        "/* Reference to rdtsc */"
        :
        : "i"(__builtin_ia32_rdtsc)
    );
#endif
}

/* Complex control flow with builtin usage */
static int complex_builtin_logic(int iterations) {
    volatile int result = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Mix of conditions and builtin calls */
        switch (i % 4) {
            case 0:
                /* Use __builtin_popcount without declaration */
                result += __builtin_popcount(i);
                break;
            case 1:
                /* Use __builtin_clz without declaration */
                if (i > 0) {
                    result += __builtin_clz(i);
                }
                break;
            case 2:
                /* Use __builtin_ctz without declaration */
                result += __builtin_ctz(i | 1); /* Avoid undefined behavior */
                break;
            case 3:
                /* Use __builtin_ffs without declaration */
                result += __builtin_ffs(i);
                break;
        }
        
        /* Occasionally call helper functions */
        if (i % 10 == 0) {
            call_builtins_inner(i);
        }
        
        /* Use __builtin_expect in loop control */
        if (__builtin_expect(i == iterations - 1, 0)) {
            result *= 2;
        }
    }
    
    return result;
}

int main(void) {
    int i, result = 0;
    
    printf("Starting builtin declaration stress test...\n");
    
    /* Initial volatile state setup */
    global_counter = 42;
    use_builtin_a = 1;
    use_builtin_b = 0;
    use_target_specific = 1;
    
    /* Main loop with builtin references */
    for (i = 0; i < 100; i++) {
        volatile int path_selector = i % 3;
        
        /* Different paths call different undeclared builtins */
        switch (path_selector) {
            case 0:
                if (use_builtin_a) {
                    /* Call builtin without declaration */
                    __builtin_trap();
                }
                break;
                
            case 1:
                if (use_builtin_b) {
                    /* Another builtin without declaration */
                    __builtin_unreachable();
                }
                break;
                
            case 2:
                /* Target-specific builtin path */
                call_target_builtins();
                break;
        }
        
        /* Take address of builtins periodically */
        if (i % 7 == 0) {
            test_builtin_pointers();
        }
        
        /* Use inline assembly references */
        if (i % 13 == 0) {
            asm_references();
        }
        
        /* Complex logic with builtins */
        if (i % 5 == 0) {
            result += complex_builtin_logic(i % 20 + 1);
        }
        
        /* Use __builtin_constant_p to check loop variable */
        if (!__builtin_constant_p(i)) {
            global_counter++;
        }
        
        /* Use __builtin_expect for branch prediction */
        if (__builtin_expect(global_counter > 1000, 0)) {
            break;
        }
    }
    
    /* Final computation using more builtins */
    result += __builtin_popcount(global_counter);
    result += __builtin_clz(global_counter | 1); /* Avoid undefined behavior */
    
    /* Use __builtin_abs without declaration */
    result = __builtin_abs(result);
    
    printf("Result: %d\n", result);
    printf("Global counter: %d\n", global_counter);
    
    /* Prevent dead code elimination */
    if (result == 0x12345678) {
        /* This should never happen, but references more builtins */
        __builtin_trap();
        __builtin_unreachable();
    }
    
    return result & 0xFF;
}
