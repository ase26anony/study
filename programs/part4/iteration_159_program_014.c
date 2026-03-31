/* Test program for GCC reorg.cc fill_eager_delay_slots coverage */
#include <stdio.h>
#include <stdlib.h>

/* Global accumulator to prevent optimization */
volatile int global_acc = 0;

/* Optimization barrier functions */
int __attribute__((noinline)) use_value(int x) {
    return x + 1;
}

int __attribute__((noinline)) get_input(void) {
    return rand() & 0xFF;
}

/* Test function 1: MIPS target with simple jump pattern */
#ifdef __mips__
__attribute__((target("arch=mips32")))
#endif
int test_case_1(int a, int b) {
    /* Create temporaries independent of jump condition */
    int temp1 = a * 3;
    int temp2 = b + 7;
    int temp3 = 0;
    int temp4 = 0;
    
    /* Dynamic condition to prevent optimization */
    if (a > b) {
        /* Simple goto to label - should generate simplejump_p */
        if ((a - b) > 5) {
            goto target_label_1;
        }
        temp3 = temp1 + temp2;
    }
    
    /* Other basic blocks to create CFG complexity */
    for (int i = 0; i < 2; i++) {
        temp4 += use_value(i);
    }
    
    return temp3 + temp4;

target_label_1:
    /* Safe, non-jump instruction after label */
    /* Uses independent temporaries not involved in jump condition */
    temp3 = temp1 & 0x7F;  /* Simple bitwise operation */
    
    /* Continue execution */
    temp4 = use_value(temp3);
    return temp4;
}

/* Test function 2: SPARC target with different pattern */
#ifdef __sparc__
__attribute__((target("arch=sparc")))
#endif
int test_case_2(int x, int y) {
    volatile int v = get_input();
    int local1 = x * y;
    int local2 = x + y;
    int local3 = 0;
    int local4 = 0;
    
    /* Multiple conditions to create branching */
    if (v > 100) {
        local3 = local1 - local2;
    } else if (v > 50) {
        /* Another simple jump candidate */
        if (x != y) {
            goto target_label_2;
        }
        local3 = local1 | local2;
    }
    
    /* Additional operations */
    local4 = use_value(local3);
    
    return local3 + local4;

target_label_2:
    /* Safe arithmetic after label */
    local3 = local2 * 2;  /* Simple multiplication */
    
    /* Use result to prevent elimination */
    local4 = use_value(local3);
    return local3 - local4;
}

/* Test function 3: Generic pattern with multiple temporaries */
int test_case_3(int p, int q) {
    /* Many independent temporaries */
    int t1 = p + 1;
    int t2 = q - 1;
    int t3 = p * q;
    int t4 = p ^ q;
    int t5 = 0;
    int t6 = 0;
    
    /* Complex enough condition to not be optimized away */
    volatile int cond = get_input();
    
    if ((p % 2) == 0) {
        t5 = t1 + t2;
        if (cond & 0x1) {
            goto target_label_3;
        }
        t6 = t3 - t4;
    } else {
        t5 = t1 | t2;
    }
    
    /* Loop to create more scheduling opportunities */
    for (int i = 0; i < 3; i++) {
        t6 += i;
    }
    
    return t5 + t6;

target_label_3:
    /* Safe logical operation after label */
    t5 = t3 & 0xFF;  /* Mask operation - cannot trap */
    
    /* Continue with safe operations */
    t6 = use_value(t5);
    return t5 ^ t6;
}

/* Test function 4: Nested control flow */
int test_case_4(int n) {
    int a = n;
    int b = n * 2;
    int c = n + 5;
    int d = 0;
    int e = 0;
    
    /* Multiple levels of conditionals */
    if (n > 0) {
        for (int i = 0; i < n && i < 3; i++) {
            a += i;
        }
        
        if (a < b) {
            if (c > a) {
                /* Target jump pattern */
                volatile int check = get_input();
                if (check % 3 == 0) {
                    goto target_label_4;
                }
                d = a + b;
            }
        }
        e = use_value(d);
    }
    
    return a + b + c + d + e;

target_label_4:
    /* Safe shift operation */
    d = b << 1;  /* Cannot trap */
    
    /* Use the result */
    e = use_value(d);
    return c + d + e;
}

/* Test function 5: Minimal pattern focusing on the exact condition */
int test_case_5(int val) {
    /* Minimal set of temporaries */
    int x = val;
    int y = val + 10;
    int z = 0;
    
    /* Very simple condition leading to goto */
    volatile int trigger = get_input();
    
    if (trigger > 128) {
        /* This should be the primary candidate for delay slot filling */
        if (x < y) {
            goto exact_target;
        }
        z = x - y;
    }
    
    return z;

exact_target:
    /* Single, simple, safe instruction after label */
    z = y + 5;  /* Simple addition - safe and movable */
    
    return z;
}

int main(void) {
    int results[5];
    int checksum = 0;
    
    /* Initialize random seed for dynamic conditions */
    srand(42);
    
    /* Execute all test cases */
    results[0] = test_case_1(10, 5);
    results[1] = test_case_2(7, 13);
    results[2] = test_case_3(6, 9);
    results[3] = test_case_4(4);
    results[4] = test_case_5(20);
    
    /* Compute checksum to verify execution */
    for (int i = 0; i < 5; i++) {
        checksum += results[i];
        global_acc += results[i];  /* Side effect to prevent optimization */
        printf("Test %d: %d\n", i + 1, results[i]);
    }
    
    printf("Checksum: %d\n", checksum);
    printf("Global accumulator: %d\n", global_acc);
    
    return (checksum > 0) ? 0 : 1;
}
