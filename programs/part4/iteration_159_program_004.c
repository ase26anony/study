/* Test program for GCC reorg.cc delay slot filling coverage */
#include <stdio.h>
#include <stdlib.h>

/* Global accumulator to prevent optimization */
volatile int global_accumulator = 0;

/* Optimization barrier functions */
static int __attribute__((noinline)) barrier(int x) {
    volatile int v = x;
    return v;
}

static void __attribute__((noinline)) use_value(int val) {
    global_accumulator += val;
}

/* Test function 1: Basic pattern for MIPS */
#ifdef __mips__
__attribute__((target("arch=mips32")))
#endif
static int test_case_1(int a, int b) {
    /* Local temporaries independent of condition */
    int temp1 = barrier(a);
    int temp2 = barrier(b);
    int temp3 = 0;
    int temp4 = 0;
    
    /* Simple condition using input-dependent variables */
    if (temp1 > temp2) {
        /* This should generate a simple jump to label */
        goto target_label_1;
    }
    
    /* Some other code to create basic blocks */
    temp3 = temp1 + 1;
    goto after_label_1;
    
target_label_1:
    /* Safe, non-jump instruction immediately after label */
    /* Uses independent temporaries not used in condition */
    temp4 = temp2 & 0xFF;  /* Simple bitwise operation */
    
after_label_1:
    /* Use the result to prevent dead code elimination */
    int result = temp3 + temp4;
    use_value(result);
    return result;
}

/* Test function 2: Different operation pattern */
#ifdef __sparc__
__attribute__((target("arch=sparc")))
#endif
static int test_case_2(int x, int y) {
    /* More temporaries to avoid resource conflicts */
    int t1 = barrier(x);
    int t2 = barrier(y);
    int t3 = t1 * 2;
    int t4 = 0;
    int t5 = 0;
    
    /* Different condition */
    if ((t1 ^ t2) != 0) {
        goto target_label_2;
    }
    
    t4 = t3 - 5;
    goto after_label_2;
    
target_label_2:
    /* Different safe operation: shift */
    t5 = t2 << 2;  /* No trap possible */
    
after_label_2:
    int result = t4 | t5;
    use_value(result);
    return result;
}

/* Test function 3: Multiple basic blocks around target */
static int test_case_3(int p, int q) {
    int a = barrier(p);
    int b = barrier(q);
    int c = 0, d = 0, e = 0;
    
    /* Some preceding basic blocks */
    if (a > 100) {
        c = a - 50;
    } else {
        c = a + 50;
    }
    
    /* The target jump pattern */
    if (b != 0) {
        goto target_label_3;
    }
    
    d = c * 3;
    goto after_label_3;
    
target_label_3:
    /* Safe arithmetic with constant */
    e = b + 17;  /* No division, no memory access */
    
after_label_3:
    /* More code after to create scheduling context */
    for (int i = 0; i < 2; i++) {
        d += i;
    }
    
    int result = d + e;
    use_value(result);
    return result;
}

/* Test function 4: Nested control flow */
static int test_case_4(int val) {
    int x = barrier(val);
    int y = 0, z = 0, w = 0;
    
    /* Loop before the target pattern */
    for (int i = 0; i < 3; i++) {
        y += i;
    }
    
    /* The critical pattern */
    if (x % 2 == 0) {  /* Even check - safe with integers */
        goto target_label_4;
    }
    
    z = y * 2;
    goto after_label_4;
    
target_label_4:
    /* Safe logical operation */
    w = x | 0x01;
    
after_label_4:
    int result = z ^ w;
    use_value(result);
    return result;
}

/* Test function 5: Multiple independent temporaries */
static int test_case_5(int m, int n) {
    /* Many independent variables to avoid resource conflicts */
    int v1 = barrier(m);
    int v2 = barrier(n);
    int v3 = v1 + v2;
    int v4 = v1 - v2;
    int v5 = 0, v6 = 0, v7 = 0, v8 = 0;
    
    /* Complex enough condition to not be optimized away */
    if (v3 > v4 && v1 != 0) {
        goto target_label_5;
    }
    
    v5 = v3 * v4;
    goto after_label_5;
    
target_label_5:
    /* Multiple independent operations that could be scheduled */
    v6 = v2 + 10;
    v7 = v1 - 5;
    v8 = v6 & v7;  /* The instruction after label */
    
after_label_5:
    int result = v5 + v8;
    use_value(result);
    return result;
}

/* Main driver */
int main(int argc, char *argv[]) {
    int seed = (argc > 1) ? atoi(argv[1]) : 12345;
    int sum = 0;
    
    /* Call test cases with different inputs */
    sum += test_case_1(seed, seed + 1);
    sum += test_case_2(seed * 2, seed / 2);
    sum += test_case_3(seed % 100, seed % 50);
    sum += test_case_4(seed ^ 0x55);
    sum += test_case_5(seed + 100, seed - 50);
    
    printf("Result checksum: %d (global accumulator: %d)\n", 
           sum, global_accumulator);
    
    return (sum != 0) ? 0 : 1;
}
