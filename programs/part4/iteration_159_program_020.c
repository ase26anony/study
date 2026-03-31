/* Test program for GCC reorg.cc fill_eager_delay_slots uncovered lines */
#include <stdio.h>
#include <stdlib.h>

/* Global accumulator to prevent optimization */
volatile int global_acc = 0;

/* Optimization barrier functions */
static int __attribute__((noinline)) barrier(int x) {
    return x ^ 0x55AA55AA;
}

static void __attribute__((noinline)) use_value(int x) {
    global_acc += x;
}

/* Test function 1: MIPS target with simple goto pattern */
#ifdef __mips__
__attribute__((target("arch=mips32")))
#endif
static int test_case_1(int a, int b) {
    /* Local temporaries independent of condition */
    int temp1 = a * 3;
    int temp2 = b + 7;
    int temp3 = 0;
    int temp4 = 0;
    
    /* Create non-trivial condition */
    if (a > b) {
        /* Use barrier to prevent constant propagation */
        temp1 = barrier(temp1);
        goto target_label_1;
    }
    
    /* Some other code to create basic blocks */
    temp2 = temp2 * 2;
    
target_label_1:
    /* Safe, non-jump instruction after label */
    temp3 = temp1 + temp2;  /* Simple arithmetic */
    
    /* Use result to prevent dead code elimination */
    return temp3 & 0xFF;
}

/* Test function 2: SPARC target with different pattern */
#ifdef __sparc__
__attribute__((target("arch=sparc")))
#endif
static int test_case_2(int x, int y) {
    volatile int v = x;  /* Prevent optimization */
    int local_a = v;
    int local_b = y;
    int local_c = 0;
    int local_d = 0;
    
    /* Different condition pattern */
    if ((x & 1) != (y & 1)) {
        local_a = local_a | 0x1000;
        goto target_label_2;
    }
    
    /* Alternative path */
    local_b = local_b << 1;
    
target_label_2:
    /* Safe instruction: bitwise operation */
    local_c = local_a ^ local_b;
    
    return local_c;
}

/* Test function 3: Generic pattern with multiple temporaries */
static int test_case_3(int p, int q) {
    int t1 = p + 1;
    int t2 = q - 1;
    int t3 = 0;
    int t4 = 0;
    int t5 = 0;
    
    /* Complex enough condition */
    if (p != q && p > 0) {
        t4 = t1 * t2;
        goto target_label_3;
    }
    
    /* Other computation */
    t5 = t1 | t2;
    
target_label_3:
    /* Safe: multiplication with constants */
    t3 = t4 * 3;
    
    return t3;
}

/* Test function 4: Nested control flow */
static int test_case_4(int val) {
    int a = val;
    int b = a + 10;
    int c = 0;
    int d = 0;
    
    /* Loop to create more complex CFG */
    for (int i = 0; i < 3; i++) {
        a += i;
    }
    
    /* Multiple conditions */
    if (val > 100) {
        if (val < 200) {
            b = barrier(b);
            goto target_label_4;
        }
    }
    
    c = a * 2;
    
target_label_4:
    /* Safe: shift operation */
    d = b << 2;
    
    return d;
}

/* Test function 5: More complex but still safe instruction */
static int test_case_5(int x, int y, int z) {
    int tmp1 = x;
    int tmp2 = y;
    int tmp3 = z;
    int result = 0;
    
    /* Use all parameters to make condition non-trivial */
    if (x + y > z) {
        tmp1 = tmp1 & 0xFF;
        tmp2 = tmp2 | 0x80;
        goto target_label_5;
    }
    
    tmp3 = tmp3 ^ 0x7F;
    
target_label_5:
    /* Multiple safe operations in sequence */
    result = tmp1 + tmp2;
    result = result - tmp3;
    
    return result;
}

/* Main driver */
int main(int argc, char *argv[]) {
    int seed = 12345;
    int result = 0;
    
    /* Use command line or fixed seed */
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Initialize with seed to make values dynamic */
    int a = seed;
    int b = seed * 2;
    int c = seed / 3;
    
    /* Run all test cases */
    result ^= test_case_1(a, b);
    use_value(result);
    
    result ^= test_case_2(b, c);
    use_value(result);
    
    result ^= test_case_3(c, a);
    use_value(result);
    
    result ^= test_case_4(a + b);
    use_value(result);
    
    result ^= test_case_5(a, b, c);
    use_value(result);
    
    /* Print checksum */
    printf("Result checksum: %d (global_acc: %d)\n", result, global_acc);
    
    return 0;
}
