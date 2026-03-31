#include <stdio.h>
#include <stdlib.h>

/* Global accumulator to prevent optimization */
volatile int global_accumulator = 0;

/* Optimization barrier functions */
int __attribute__((noinline)) use_value(int x) {
    return x ^ 0x55AA55AA;
}

int __attribute__((noinline)) get_input(void) {
    static volatile int counter = 0;
    return ++counter;
}

/* Test function for MIPS target */
#ifdef __mips__
__attribute__((target("arch=mips32")))
#endif
int test_case_1(int a, int b) {
    /* Create temporaries independent of condition */
    int temp1 = a * 3;
    int temp2 = b + 7;
    int temp3 = 0;
    
    /* Dynamic condition using arguments */
    if (a > b) {
        /* This should compile to a simple jump to label */
        goto target_label_1;
    }
    
    /* Some other code to create CFG complexity */
    temp1 = use_value(temp1);
    
    /* This should never be reached if a > b */
    return temp1 + temp2;
    
target_label_1:
    /* Safe, non-jump instruction after label */
    temp3 = temp1 & 0xFF;  /* Simple bitwise operation */
    
    /* Use the result to prevent elimination */
    global_accumulator += temp3;
    return temp3;
}

/* Test function for SPARC target */
#ifdef __sparc__
__attribute__((target("arch=sparc")))
#endif
int test_case_2(int x, int y) {
    /* Independent temporaries */
    int local_a = x * 2;
    int local_b = y - 5;
    int local_c = 0;
    int local_d = 0;
    
    /* Create more complex control flow */
    for (int i = 0; i < 3; i++) {
        local_a += i;
    }
    
    /* Dynamic condition */
    if (x != 0 && y % 2 == 0) {
        goto target_label_2;
    }
    
    /* Alternative path */
    local_b = use_value(local_b);
    return local_b;
    
target_label_2:
    /* Safe instruction after label - arithmetic only */
    local_c = local_a | 0x01;  /* Simple OR operation */
    
    /* Another safe operation */
    local_d = local_c ^ local_b;
    
    global_accumulator += local_d;
    return local_d;
}

/* Generic test function */
int test_case_3(int val) {
    volatile int v = val;  /* Prevent constant propagation */
    int t1 = v + 100;
    int t2 = 50;
    int t3 = 0;
    
    /* Condition using volatile read */
    if (v > 50) {
        goto target_label_3;
    }
    
    /* Different basic block */
    t2 = t1 * 2;
    return t2;
    
target_label_3:
    /* Safe: shift operation on local */
    t3 = t1 << 2;  /* Simple shift */
    
    /* Use result */
    global_accumulator ^= t3;
    return t3;
}

/* Test with multiple labels and jumps */
int test_case_4(int a, int b, int c) {
    int x = a * b;
    int y = c + 10;
    int z = 0;
    
    /* Nested conditions */
    if (a > 0) {
        if (b < 0) {
            goto inner_label;
        }
        x = use_value(x);
    }
    
    if (c == 0) {
        goto outer_label;
    }
    
    return x + y;
    
inner_label:
    /* Safe instruction - multiplication */
    z = x * y;  /* Only uses locals defined before any jump */
    
    /* Continue to outer label */
    goto outer_label;
    
outer_label:
    /* Another safe instruction */
    z = z & 0xFFFF;
    
    global_accumulator += z;
    return z;
}

/* Main test driver */
int main(void) {
    int results[4];
    int checksum = 0;
    
    /* Initialize with different values */
    int inputs[] = {10, 5, 60, -3, 8, 25};
    
    /* Run test cases */
    results[0] = test_case_1(inputs[0], inputs[1]);
    results[1] = test_case_2(inputs[2], inputs[3]);
    results[2] = test_case_3(inputs[4]);
    results[3] = test_case_4(inputs[0], inputs[1], inputs[5]);
    
    /* Calculate checksum */
    for (int i = 0; i < 4; i++) {
        checksum ^= results[i];
        checksum = (checksum << 1) | (checksum >> 31);  /* Rotate */
    }
    
    checksum += global_accumulator;
    
    printf("Test results: %d, %d, %d, %d\n", 
           results[0], results[1], results[2], results[3]);
    printf("Global accumulator: %d\n", global_accumulator);
    printf("Checksum: 0x%08X\n", checksum);
    
    return 0;
}
