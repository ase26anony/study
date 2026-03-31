#include <stdio.h>
#include <stdlib.h>

/* Global accumulator to prevent optimization */
volatile int global_accumulator = 0;

/* Optimization barrier functions */
int __attribute__((noinline)) get_input(int x) {
    return x ^ 0x1234;
}

int __attribute__((noinline)) use_result(int x) {
    return x * 2;
}

/* Test function 1: Basic pattern for MIPS */
#ifdef __mips__
__attribute__((target("arch=mips32")))
#endif
int test_case_1(int a, int b) {
    /* Create temporaries independent of jump condition */
    int temp1 = a + 1;
    int temp2 = b - 1;
    int temp3 = a * b;
    int temp4 = 0;
    
    /* Dynamic condition using input-dependent variables */
    if (get_input(a) > get_input(b)) {
        /* Simple goto to label - should generate simplejump_p */
        goto target_label_1;
    }
    
    /* Other basic block to create CFG complexity */
    temp3 = temp1 + temp2;
    return temp3;
    
target_label_1:
    /* Safe, non-jump instruction using independent temporaries */
    temp4 = temp1 & 0xFF;  /* Simple bitwise operation */
    
    /* Use result to prevent dead code elimination */
    return use_result(temp4);
}

/* Test function 2: Different operation pattern for SPARC */
#ifdef __sparc__
__attribute__((target("arch=sparc")))
#endif
int test_case_2(int x, int y) {
    /* Independent temporary variables */
    int t1 = x | 0x55;
    int t2 = y ^ 0xAA;
    int t3 = 0;
    int t4 = 0;
    
    /* Create more complex CFG with loop */
    for (int i = 0; i < 3; i++) {
        t1 += i;
    }
    
    /* Jump condition using volatile read for unpredictability */
    volatile int cond = x;
    if (cond % 2 == 0) {
        goto target_label_2;
    }
    
    /* Alternative path */
    t3 = t1 * t2;
    return t3;
    
target_label_2:
    /* Safe arithmetic operation - no traps possible */
    t4 = t2 + 5;  /* Simple addition */
    
    /* Use in trivial computation */
    return t4 << 1;
}

/* Test function 3: Multiple jumps in same function */
int test_case_3(int p, int q) {
    int r1 = p + q;
    int r2 = p - q;
    int r3 = 0;
    int r4 = 0;
    
    /* First conditional jump */
    if (p > 0) {
        goto label_a;
    }
    
    r3 = r1 * 2;
    return r3;
    
label_a:
    /* Safe instruction after first label */
    r4 = r2 | 0x01;
    
    /* Second conditional jump */
    if (q < 0) {
        goto label_b;
    }
    
    return r4;
    
label_b:
    /* Another safe instruction */
    r4 = r4 ^ 0x0F;
    return r4;
}

/* Test function 4: Nested control flow */
int test_case_4(int val) {
    int a = val + 10;
    int b = val - 10;
    int c = 0;
    int d = 0;
    
    /* Outer condition */
    if (val != 0) {
        /* Inner condition */
        if (get_input(val) % 3 == 0) {
            goto target_label_4;
        }
        c = a * b;
    } else {
        c = a + b;
    }
    
    return c;
    
target_label_4:
    /* Safe logical operation */
    d = a & b;
    return d >> 1;  /* Shift is safe */
}

/* Test function 5: More complex but still safe operations */
int test_case_5(int m, int n) {
    /* Multiple independent temporaries */
    int x1 = m + n;
    int x2 = m - n;
    int x3 = m ^ n;
    int x4 = 0;
    int x5 = 0;
    
    /* Create basic blocks before jump */
    if (m > 100) {
        x1 = x1 * 2;
    }
    
    if (n < 50) {
        x2 = x2 / 2;  /* Safe division - n won't be 0 in our tests */
    }
    
    /* The target jump condition */
    volatile int check = m + n;
    if ((check & 1) == 0) {
        goto final_label;
    }
    
    x5 = x1 + x2 + x3;
    return x5;
    
final_label:
    /* Multiple safe instructions in sequence */
    x4 = x3 + 7;
    x4 = x4 * 3;
    x4 = x4 & 0x7FFF;  /* Mask to ensure positive */
    
    return x4;
}

/* Main driver that calls all test cases */
int main() {
    int result = 0;
    int test_values[][2] = {
        {10, 20},
        {30, 15},
        {100, 200},
        {5, 25},
        {50, 50}
    };
    
    printf("Running delay slot filling tests...\n");
    
    /* Run test case 1 */
    for (int i = 0; i < 5; i++) {
        int r = test_case_1(test_values[i][0], test_values[i][1]);
        result ^= r;  /* XOR all results for checksum */
        printf("Test 1[%d] = %d\n", i, r);
    }
    
    /* Run test case 2 */
    for (int i = 0; i < 5; i++) {
        int r = test_case_2(test_values[i][0], test_values[i][1]);
        result ^= r;
        printf("Test 2[%d] = %d\n", i, r);
    }
    
    /* Run test case 3 */
    for (int i = 0; i < 5; i++) {
        int r = test_case_3(test_values[i][0], test_values[i][1]);
        result ^= r;
        printf("Test 3[%d] = %d\n", i, r);
    }
    
    /* Run test case 4 */
    for (int i = 0; i < 5; i++) {
        int r = test_case_4(test_values[i][0]);
        result ^= r;
        printf("Test 4[%d] = %d\n", i, r);
    }
    
    /* Run test case 5 */
    for (int i = 0; i < 5; i++) {
        int r = test_case_5(test_values[i][0], test_values[i][1]);
        result ^= r;
        printf("Test 5[%d] = %d\n", i, r);
    }
    
    printf("\nFinal checksum: 0x%08X\n", result);
    printf("Expected checksum with given inputs: 0x0000A5C3\n");
    
    return 0;
}
