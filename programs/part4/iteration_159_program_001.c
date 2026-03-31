/* Test program to trigger delay slot filling logic in reorg.cc */
#include <stdio.h>
#include <stdlib.h>

/* Global accumulator to prevent optimization */
volatile int global_acc = 0;

/* Optimization barrier functions */
int __attribute__((noinline)) use_value(int x) {
    return x ^ 0x55AA55AA;
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
    int temp3 = temp1 ^ temp2;
    int result = 0;
    
    /* Dynamic condition to prevent optimization */
    if (a > b) {
        /* Simple jump to label */
        if ((a - b) > 10) {
            goto target_label_1;
        }
        result = temp3 + 1;
    } else {
        result = temp3 - 1;
    }
    
    /* Some other code to create CFG complexity */
    for (int i = 0; i < 2; i++) {
        result += i;
    }
    
    return use_value(result);

target_label_1:
    /* Safe, non-jump instruction after label */
    temp3 = temp1 & 0x7F;  /* Simple bitwise operation */
    result = temp3 + 100;
    
    /* Use result to prevent dead code elimination */
    return use_value(result);
}

/* Test function 2: SPARC target with different pattern */
#ifdef __sparc__
__attribute__((target("arch=sparc")))
#endif
int test_case_2(int x, int y) {
    /* Independent temporaries */
    int local_a = x * 5;
    int local_b = y * 3;
    int local_c = local_a | local_b;
    int local_d = local_a ^ local_b;
    int result = 0;
    
    /* More complex condition but still simple jump */
    volatile int cond = get_input();
    if (x != 0 && y != 0) {
        if (cond > 128) {
            goto target_label_2;
        }
        result = local_c * 2;
    }
    
    /* Additional basic blocks */
    if (x < y) {
        result += local_d;
    } else {
        result -= local_d;
    }
    
    return use_value(result);

target_label_2:
    /* Safe arithmetic after label */
    local_d = local_c + local_b;  /* Simple addition */
    result = local_d * 3;
    
    return use_value(result);
}

/* Test function 3: Generic pattern with multiple jumps */
int test_case_3(int p, int q) {
    /* Multiple independent variables */
    int var1 = p + q;
    int var2 = p - q;
    int var3 = var1 * var2;
    int var4 = var1 ^ var2;
    int result = 0;
    
    /* Nested conditions leading to simple jump */
    if (p > 0) {
        if (q > 0) {
            if ((p + q) < 1000) {
                /* This should be a simple jump */
                if (var3 > var4) {
                    goto target_label_3;
                }
                result = var3;
            } else {
                result = var4;
            }
        } else {
            result = var1;
        }
    } else {
        result = var2;
    }
    
    /* Loop to create scheduling opportunities */
    for (int i = 0; i < 3; i++) {
        result += i * 2;
    }
    
    return use_value(result);

target_label_3:
    /* Safe logical operation after label */
    var4 = var3 & 0xFF;  /* Mask operation */
    result = var4 + 50;
    
    return use_value(result);
}

/* Test function 4: Pattern with volatile to prevent optimization */
int test_case_4(void) {
    volatile int v1 = get_input();
    volatile int v2 = get_input();
    
    int t1 = v1 + 1;
    int t2 = v2 + 2;
    int t3 = t1 * t2;
    int t4 = t1 | t2;
    int result = 0;
    
    /* Unpredictable condition */
    if (v1 != v2) {
        if ((v1 > 50 && v2 < 200) || (v1 < v2)) {
            goto target_label_4;
        }
        result = t3;
    }
    
    /* Additional operations */
    result = result ^ t4;
    
    return use_value(result);

target_label_4:
    /* Safe shift operation */
    t4 = t3 << 2;  /* Simple shift */
    result = t4 - 10;
    
    return use_value(result);
}

/* Main driver */
int main(void) {
    int checksum = 0;
    
    /* Seed RNG for get_input() */
    srand(42);
    
    /* Execute all test cases with various inputs */
    checksum ^= test_case_1(100, 50);   /* Should take jump */
    checksum ^= test_case_1(50, 100);   /* Should not take jump */
    checksum ^= test_case_2(75, 25);
    checksum ^= test_case_2(25, 75);
    checksum ^= test_case_3(30, 40);
    checksum ^= test_case_3(-10, 20);
    checksum ^= test_case_4();
    checksum ^= test_case_4();  /* Call twice with different volatile values */
    
    /* Also test with global accumulator */
    global_acc = checksum;
    
    printf("Checksum: 0x%08X\n", checksum);
    printf("Global accumulator: %d\n", global_acc);
    
    return (checksum != 0) ? 0 : 1;
}
