#include <stdio.h>
#include <stdlib.h>

volatile int global_seed = 42;
int global_accumulator = 0;

/* Optimization barrier */
int __attribute__((noinline)) get_input(int x) {
    return x + global_seed;
}

/* MIPS-specific test function */
#ifdef __mips__
__attribute__((target("arch=mips32")))
#endif
int test_case_1(int a, int b) {
    /* Create temporaries that are independent of the jump condition */
    int temp1 = a * 3;
    int temp2 = b + 7;
    int temp3 = temp1 ^ temp2;
    
    /* Simple conditional jump to label */
    if (a > b) {
        /* Use volatile to prevent constant folding */
        volatile int cond = get_input(a);
        if (cond != 0) {
            goto target_label_1;
        }
    }
    
    /* Some other code to create basic blocks */
    temp3 = temp3 * 2;
    
    /* The target label with a safe instruction immediately after */
target_label_1:
    /* Safe, non-jump instruction: simple arithmetic on temporaries */
    temp3 = temp1 + temp2;  /* This should be the 'next_trial' instruction */
    
    /* Use the result to prevent dead code elimination */
    return temp3 + 1;
}

/* SPARC-specific test function */
#ifdef __sparc__
__attribute__((target("arch=sparc")))
#endif
int test_case_2(int x, int y) {
    /* Independent temporary variables */
    int local_a = x & 0xFF;
    int local_b = y | 0x55;
    int local_c = local_a - local_b;
    
    /* Create multiple basic blocks */
    for (int i = 0; i < 2; i++) {
        local_c += i;
    }
    
    /* Conditional jump with non-trivial condition */
    volatile int check = get_input(x);
    if ((check & 1) && (x != y)) {
        goto target_label_2;
    }
    
    /* Alternative path */
    local_c = local_c >> 1;
    
target_label_2:
    /* Safe instruction: bitwise operation on locals */
    local_c = local_a ^ local_b;  /* Should be eligible for delay slot */
    
    /* Barrier to prevent reordering */
    asm volatile("" : : "r"(local_c));
    
    return local_c * 3;
}

/* Generic test function for architectures with delay slots */
int test_case_3(int p, int q) {
    /* Multiple independent temporaries */
    int t1 = p + 100;
    int t2 = q - 50;
    int t3 = t1 * 2;
    int t4 = t2 / 3;  /* Division is safe here as divisor is constant non-zero */
    
    /* Complex enough condition to avoid optimization */
    volatile int v = get_input(p + q);
    if ((p > 0) && (q < 100) && (v > 50)) {
        /* Nested condition to create more basic blocks */
        if (t3 != t4) {
            goto target_label_3;
        }
    }
    
    /* Different computation path */
    t3 = t4 + t1;
    
target_label_3:
    /* Safe instruction: simple assignment with arithmetic */
    t4 = t3 + t2;  /* Candidate for delay slot filling */
    
    /* Use result in computation */
    int result = t4;
    for (int i = 0; i < 3; i++) {
        result += i;
    }
    
    return result;
}

/* Test with multiple jumps and labels */
int test_case_4(int val) {
    int a = val * 2;
    int b = val + 10;
    int c = 0;
    
    /* First conditional jump */
    if (val % 3 == 0) {
        volatile int cond = get_input(val);
        if (cond & 1) {
            goto label_a;
        }
    }
    
    /* Second conditional jump */
    if (val % 5 == 0) {
        c = a + b;
        goto label_b;
    }
    
    c = a - b;
    goto label_c;
    
label_a:
    /* Safe instruction after first label */
    c = a * b;  /* Potential delay slot candidate */
    goto label_c;
    
label_b:
    /* Another safe instruction */
    c = b - a;  /* Another candidate */
    
label_c:
    /* Final computation */
    return c + val;
}

/* Main driver that calls all test cases */
int main() {
    int results[4];
    int checksum = 0;
    
    /* Test with various inputs to explore different paths */
    results[0] = test_case_1(10, 5);
    results[1] = test_case_2(20, 30);
    results[2] = test_case_3(40, 60);
    results[3] = test_case_4(25);
    
    /* Compute checksum to verify execution */
    for (int i = 0; i < 4; i++) {
        checksum += results[i];
        printf("Test %d result: %d\n", i + 1, results[i]);
    }
    
    printf("Total checksum: %d\n", checksum);
    
    /* Also test with different inputs to trigger different branches */
    global_accumulator += test_case_1(5, 10);
    global_accumulator += test_case_2(100, 200);
    global_accumulator += test_case_3(-10, 20);
    global_accumulator += test_case_4(15);
    
    printf("Global accumulator: %d\n", global_accumulator);
    
    return checksum != 0 ? 0 : 1;
}
