/* Test program for reorg.cc uncovered lines 2135-2149 */
#include <stdio.h>
#include <stdlib.h>

volatile int global_seed = 42;
int global_accumulator = 0;

/* Optimization barrier */
int __attribute__((noinline)) get_value(int x) {
    return x ^ global_seed;
}

/* MIPS-specific variant */
#ifdef __mips__
__attribute__((target("arch=mips32")))
#endif
int test_case_1(int a, int b) {
    /* Create temporaries independent of condition */
    int temp1 = a * 3;
    int temp2 = b + 7;
    int temp3 = 0;
    
    /* Dynamic condition using function arguments */
    if (get_value(a) > get_value(b)) {
        /* This should compile to a simple jump to label */
        goto target_label_1;
    }
    
    /* Some other code to create basic blocks */
    temp1 = temp1 * 2;
    return temp1 + temp2;
    
target_label_1:
    /* Safe, non-jump instruction after label */
    temp3 = temp1 & 0xFF;  /* Simple bitwise operation */
    return temp3 + temp2;
}

/* SPARC-specific variant */
#ifdef __sparc__
__attribute__((target("arch=sparc")))
#endif
int test_case_2(int x, int y) {
    /* Independent temporaries */
    int local_a = x << 2;
    int local_b = y | 0x55;
    int local_c = 0;
    
    /* Volatile read to prevent optimization */
    volatile int v = global_seed;
    if ((x ^ v) < (y ^ v)) {
        goto target_label_2;
    }
    
    /* Alternative path */
    local_a = local_a + 100;
    return local_a - local_b;
    
target_label_2:
    /* Safe arithmetic operation after label */
    local_c = local_b + local_a;  /* Simple addition */
    return local_c * 2;
}

/* Generic variant for delay slot architectures */
int test_case_3(int p, int q) {
    /* Multiple independent variables */
    int t1 = p + q;
    int t2 = p - q;
    int t3 = p * 2;
    int t4 = 0;
    
    /* Non-trivial condition */
    if ((p & 1) && (q & 2)) {
        goto target_label_3;
    }
    
    /* Other basic block */
    t1 = t1 ^ t2;
    return t1 | t3;
    
target_label_3:
    /* Safe logical operation */
    t4 = t3 ^ 0xAA;  /* XOR with constant */
    return t4 + t2;
}

/* More complex control flow with multiple jumps */
int test_case_4(int val) {
    int a = val * 3;
    int b = val + 5;
    int c = 0;
    
    /* Nested conditions to create more CFG complexity */
    if (val > 10) {
        if (val < 20) {
            goto target_label_4;
        }
        a = a + 100;
    }
    
    b = b * 2;
    return a + b;
    
target_label_4:
    /* Safe shift operation */
    c = b << 1;  /* Multiply by 2 */
    return c - a;
}

/* Function with loop to create scheduling context */
int test_case_5(int n) {
    int sum = 0;
    int i;
    
    for (i = 0; i < n; i++) {
        int temp_a = i * 2;
        int temp_b = i + 1;
        
        if (temp_a > temp_b) {
            goto target_label_5;
        }
        
        sum += temp_a;
        continue;
        
    target_label_5:
        /* Safe operation in delay slot candidate */
        temp_b = temp_a & 0xF;  /* Mask operation */
        sum += temp_b;
    }
    
    return sum;
}

int main() {
    int result = 0;
    
    /* Call test functions with varying inputs */
    result += test_case_1(5, 10);
    result += test_case_2(15, 25);
    result += test_case_3(7, 3);
    result += test_case_4(15);
    result += test_case_5(5);
    
    /* Add to global accumulator to prevent optimization */
    global_accumulator += result;
    
    printf("Result checksum: %d\n", result);
    printf("Global accumulator: %d\n", global_accumulator);
    
    return 0;
}
