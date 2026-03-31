/* Test program for triggering delay slot filling logic in GCC's reorg pass */
#include <stdio.h>
#include <stdlib.h>

/* Global accumulator to prevent optimization */
volatile int global_acc = 0;

/* Optimization barrier functions */
static int __attribute__((noinline)) barrier(int x) {
    return x ^ 0x55AA55AA;
}

static void __attribute__((noinline)) use_value(int val) {
    global_acc += val;
}

/* Test function 1: MIPS target with simple conditional jump */
#ifdef __mips__
__attribute__((target("arch=mips32")))
#endif
static int test_case_1(int a, int b) {
    /* Local temporaries independent of condition */
    int temp1 = a + 1;
    int temp2 = b * 2;
    int temp3 = 0;
    int temp4 = 0;
    
    /* Create a non-trivial condition */
    if (a > b && (a ^ b) != 0) {
        /* Simple goto to label - should generate simplejump_p */
        goto target_label_1;
    }
    
    /* Other basic blocks to create CFG complexity */
    for (int i = 0; i < 3; i++) {
        temp1 += i;
    }
    
    return temp1 + temp2;

target_label_1:
    /* Safe, non-jump instruction after label */
    /* Uses independent temporaries not used in condition */
    temp3 = temp1 & 0xFF;  /* Simple bitwise operation */
    temp4 = temp3 + 5;     /* Simple arithmetic */
    
    /* Use result to prevent elimination */
    return barrier(temp4);
}

/* Test function 2: SPARC target with different pattern */
#ifdef __sparc__
__attribute__((target("arch=sparc")))
#endif
static int test_case_2(int x, int y) {
    /* More independent temporaries */
    int local_a = x | 0x01;
    int local_b = y & 0xFE;
    int local_c = 0;
    int local_d = 0;
    
    /* Volatile read to prevent constant folding */
    volatile int v = x;
    
    /* Another non-trivial condition */
    if ((v % 7) != (y % 5)) {
        /* Another simple goto */
        goto target_label_2;
    }
    
    /* Alternative path with some computation */
    local_a = local_a * 3;
    return local_a + local_b;

target_label_2:
    /* Different safe operation after label */
    local_c = local_b ^ 0xAA;  /* Bitwise XOR with constant */
    local_d = local_c << 1;    /* Simple shift */
    
    return barrier(local_d);
}

/* Test function 3: Generic with multiple basic blocks */
static int test_case_3(int seed) {
    int t1 = seed + 100;
    int t2 = seed * 2;
    int t3 = 0;
    int t4 = 0;
    
    /* Multiple conditions to create complex CFG */
    if (seed & 1) {
        if (seed & 2) {
            /* Nested condition leading to goto */
            goto target_label_3;
        }
        t1 = t1 - 50;
    }
    
    /* Loop to add scheduling complexity */
    for (int j = 0; j < 4; j++) {
        t2 += j;
    }
    
    return t1 + t2;

target_label_3:
    /* Safe arithmetic operation */
    t3 = t1 + t2;      /* Uses both temporaries */
    t4 = t3 * 2;       /* Simple multiplication */
    
    return barrier(t4);
}

/* Test function 4: More complex but still safe operations */
static int test_case_4(int p, int q) {
    int var1 = p ^ q;
    int var2 = p & q;
    int var3 = 0;
    int result = 0;
    
    /* Use barrier in condition to prevent optimization */
    int cond = barrier(p);
    
    if (cond > q && (p != q)) {
        goto target_label_4;
    }
    
    /* Another path */
    var1 = var1 | 0x80000000;
    return var1;

target_label_4:
    /* Multiple safe instructions (compiler might schedule one into delay slot) */
    var3 = var2 + 1;           /* Increment */
    result = var3 - var1;      /* Subtraction */
    result = result & 0x7FFF;  /* Masking */
    
    return barrier(result);
}

/* Test function 5: Minimal pattern focusing on the exact condition */
static int test_case_5(int a, int b) {
    int tmp1 = a;
    int tmp2 = b;
    int tmp3 = 0;
    
    /* Very simple condition - just different parity */
    if ((a ^ b) & 1) {
        goto minimal_target;
    }
    
    tmp1 = tmp1 * 3;
    return tmp1;

minimal_target:
    /* Single, very simple operation */
    tmp3 = tmp2 + 1;  /* Just increment */
    
    return barrier(tmp3);
}

int main(void) {
    int results[5];
    int checksum = 0;
    
    /* Initialize with non-constant values */
    volatile int input1 = 42;
    volatile int input2 = 17;
    volatile int input3 = 123;
    volatile int input4 = 255;
    volatile int input5 = 8;
    
    /* Run all test cases */
    results[0] = test_case_1(input1, input2);
    results[1] = test_case_2(input2, input3);
    results[2] = test_case_3(input3);
    results[3] = test_case_4(input4, input5);
    results[4] = test_case_5(input5, input1);
    
    /* Compute checksum and use results */
    for (int i = 0; i < 5; i++) {
        checksum ^= results[i];
        use_value(results[i]);
    }
    
    printf("Checksum: 0x%08X\n", checksum);
    printf("Global accumulator: %d\n", global_acc);
    
    return 0;
}
