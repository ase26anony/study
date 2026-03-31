#include <stdio.h>
#include <stdlib.h>

/* Global accumulator to prevent optimization */
volatile int global_accumulator = 0;

/* Optimization barrier functions */
int __attribute__((noinline)) get_input(int x) {
    return x ^ 0x55AA55AA;
}

int __attribute__((noinline)) use_result(int x) {
    global_accumulator += x;
    return x;
}

/* Test function 1: MIPS target with simple jump pattern */
#ifdef __mips__
__attribute__((target("arch=mips32")))
#endif
int test_case_1(int a, int b) {
    volatile int cond = a > b;  /* Prevent constant folding */
    int temp1 = a + 1;
    int temp2 = b * 2;
    int temp3 = 0;
    
    /* Other basic blocks to create scheduling context */
    if (a < 0) {
        temp1 = -temp1;
    }
    
    /* Target pattern: simple jump to label */
    if (cond) {
        /* This should compile to a simple jump */
        goto target_label_1;
    }
    
    /* Fall-through path */
    temp3 = temp1 + temp2;
    return use_result(temp3);
    
target_label_1:
    /* Safe, non-jump instruction after label */
    temp3 = temp1 & 0xFF;  /* Simple bitwise operation */
    
    /* Use result to prevent dead code elimination */
    return use_result(temp3);
}

/* Test function 2: SPARC target with different pattern */
#ifdef __sparc__
__attribute__((target("arch=sparc")))
#endif
int test_case_2(int x, int y) {
    volatile int flag = (x & 1);  /* Dynamic condition */
    int local_a = x + y;
    int local_b = x - y;
    int local_c = 0;
    int local_d = 0;
    
    /* Create some control flow complexity */
    for (int i = 0; i < 3; i++) {
        local_a += i;
    }
    
    /* Another basic block */
    if (y != 0) {
        local_b = local_b / 2;  /* Before the jump, safe */
    }
    
    /* Target jump pattern */
    if (flag) {
        goto target_label_2;
    }
    
    local_c = local_a | local_b;
    return use_result(local_c);
    
target_label_2:
    /* Safe instruction using independent temporaries */
    local_d = local_b ^ 0x1234;  /* Simple XOR with constant */
    
    return use_result(local_d);
}

/* Test function 3: Generic delay slot pattern */
int test_case_3(int p, int q) {
    /* Use volatile to prevent optimization */
    volatile int check = get_input(p) > get_input(q);
    int tmp1 = p * 3;
    int tmp2 = q + 7;
    int tmp3 = 0;
    int tmp4 = 0;
    
    /* Multiple basic blocks */
    if (p % 2 == 0) {
        tmp1 = tmp1 >> 1;
    } else {
        tmp1 = tmp1 << 1;
    }
    
    /* Additional computation */
    tmp2 = tmp2 & 0xFFFF;
    
    /* Target jump - should be simplejump_p */
    if (check) {
        goto target_label_3;
    }
    
    tmp3 = tmp1 + tmp2;
    return use_result(tmp3);
    
target_label_3:
    /* Safe, movable instruction */
    tmp4 = tmp2 * 2;  /* Simple multiplication by 2 (shift) */
    
    return use_result(tmp4);
}

/* Test function 4: More complex surrounding code */
int test_case_4(int val) {
    volatile int threshold = 100;
    int a = val;
    int b = val + 10;
    int c = 0;
    int d = 0;
    
    /* Loop to create scheduling opportunities */
    for (int i = 0; i < 5; i++) {
        a += i;
        if (i % 2) {
            b -= i;
        }
    }
    
    /* Multiple conditions before target jump */
    if (a > threshold) {
        if (b < threshold * 2) {
            goto target_label_4;
        }
    }
    
    c = a ^ b;
    return use_result(c);
    
target_label_4:
    /* Safe arithmetic on independent variable */
    d = b + 5;  /* Simple addition */
    
    return use_result(d);
}

/* Test function 5: Minimal pattern */
int test_case_5(int x) {
    volatile int trigger = x != 0;
    int result = 0;
    
    /* Very simple pattern */
    if (trigger) {
        goto minimal_label;
    }
    
    result = 1;
    return use_result(result);
    
minimal_label:
    result = 2;  /* Simple assignment */
    
    return use_result(result);
}

int main() {
    int checksum = 0;
    
    /* Call test functions with different inputs */
    checksum += test_case_1(10, 5);
    checksum += test_case_1(5, 10);
    
    checksum += test_case_2(7, 3);
    checksum += test_case_2(2, 8);
    
    checksum += test_case_3(15, 25);
    checksum += test_case_3(30, 10);
    
    checksum += test_case_4(50);
    checksum += test_case_4(150);
    
    checksum += test_case_5(0);
    checksum += test_case_5(42);
    
    /* Add global accumulator */
    checksum += global_accumulator;
    
    printf("Checksum: %d\n", checksum);
    
    /* Also print to prevent optimization */
    printf("Global accumulator: %d\n", global_accumulator);
    
    return 0;
}
