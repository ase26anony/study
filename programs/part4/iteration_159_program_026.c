#include <stdio.h>
#include <stdlib.h>

/* Global accumulator to prevent optimization */
volatile int global_accumulator = 0;

/* Optimization barrier functions */
__attribute__((noinline)) int get_input(int x) {
    return x ^ 0x55AA55AA;
}

__attribute__((noinline)) int use_result(int x) {
    return x + 1;
}

/* Test function 1: Basic pattern for MIPS */
__attribute__((target("arch=mips32")))
int test_mips_basic(int arg1, int arg2) {
    /* Create temporaries independent of jump condition */
    int temp1 = arg1 * 3;
    int temp2 = arg2 + 7;
    int temp3 = 0;
    
    /* Dynamic condition to prevent optimization */
    if (arg1 > arg2) {
        /* Simple jump to label */
        goto target_label_1;
    }
    
    /* Some other code to create CFG complexity */
    temp1 = arg1 - arg2;
    
target_label_1:
    /* Safe, non-jump instruction after label */
    temp3 = temp1 & 0xFF;  /* Simple bitwise operation */
    
    /* Use result to prevent dead code elimination */
    return use_result(temp3) + temp2;
}

/* Test function 2: Different operation pattern for SPARC */
__attribute__((target("arch=sparc")))
int test_sparc_variant(int base, int offset) {
    int local_a = base * 2;
    int local_b = offset / 2;  /* Division is before jump, not after label */
    int local_c = 0;
    int local_d = 100;
    
    /* Use volatile to prevent constant folding */
    volatile int vol = base;
    if (vol != offset) {
        goto sparc_target;
    }
    
    /* Alternative path */
    local_a = base + offset;
    
sparc_target:
    /* Safe arithmetic after label - uses independent variables */
    local_c = local_d - local_a;  /* Subtraction with constant */
    
    /* Create dependency to keep value alive */
    return local_c + local_b;
}

/* Test function 3: More complex CFG with multiple labels */
__attribute__((target("arch=mips32")))
int test_mips_complex(int x, int y, int z) {
    int t1 = x + y;
    int t2 = y * z;
    int t3 = 0;
    int t4 = 255;
    
    /* Nested conditions to create more CFG edges */
    if (x > 0) {
        if (y < 100) {
            if (z != 0) {
                goto mips_label;
            }
        }
    }
    
    /* Different computation path */
    t1 = x - y;
    
mips_label:
    /* Safe logical operation after label */
    t3 = t4 | t1;  /* OR operation with constant */
    
    /* Mix results */
    return t3 ^ t2;
}

/* Test function 4: Pattern with multiple temporaries */
int test_generic(int a, int b) {
    /* No target attribute - rely on compilation flags */
    int r1 = a << 2;
    int r2 = b >> 1;
    int r3 = 0x1234;
    int r4 = 0;
    
    /* Complex enough condition */
    int cond = get_input(a);
    if ((cond & 1) && (b % 3 == 0)) {
        goto generic_label;
    }
    
    r1 = a + b;
    
generic_label:
    /* Multiple safe operations in sequence */
    r4 = r3 + r1;  /* Addition */
    r4 = r4 * 2;   /* Multiplication by 2 (safe) */
    
    return r4 - r2;
}

/* Test function 5: Loop with internal jump */
__attribute__((target("arch=sparc")))
int test_sparc_loop(int iterations) {
    int sum = 0;
    int counter = 0;
    int temp = 0;
    
    for (int i = 0; i < iterations; i++) {
        counter = i * 2;
        
        /* Jump inside loop body */
        if (i & 1) {  /* Check LSB */
            goto loop_label;
        }
        
        temp = i + 10;
        continue;
        
    loop_label:
        /* Safe operation after label inside loop */
        temp = counter & 0xF;  /* Mask operation */
        
        sum += temp;
    }
    
    return sum;
}

/* Test function 6: Function call around jump pattern */
__attribute__((target("arch=mips32")))
int test_with_call(int val) {
    int pre = val * 3;
    int post = 0;
    
    /* Call acts as optimization barrier */
    int adjusted = get_input(val);
    
    if (adjusted > 1000) {
        goto call_label;
    }
    
    pre = val - 10;
    
call_label:
    /* Safe shift operation */
    post = pre << 1;
    
    return post + adjusted;
}

/* Main driver */
int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Use command line args or defaults for variability */
    int a = argc > 1 ? atoi(argv[1]) : 42;
    int b = argc > 2 ? atoi(argv[2]) : 17;
    int c = argc > 3 ? atoi(argv[3]) : 99;
    
    printf("Testing delay slot filling patterns...\n");
    
    /* Execute all test patterns */
    result ^= test_mips_basic(a, b);
    result ^= test_sparc_variant(b, c);
    result ^= test_mips_complex(a, b, c);
    result ^= test_generic(c, a);
    result ^= test_sparc_loop(5);  /* Small loop */
    result ^= test_with_call(b);
    
    /* Update global to prevent optimization */
    global_accumulator += result;
    
    printf("Result checksum: 0x%08X\n", result);
    printf("Global accumulator: %d\n", global_accumulator);
    
    return (result != 0) ? 0 : 1;
}
