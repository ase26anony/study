/* Test program to trigger delay slot filling logic in reorg.cc */
#include <stdio.h>
#include <stdlib.h>

/* Global accumulator to prevent optimization */
volatile int global_accumulator = 0;

/* Optimization barrier functions */
int __attribute__((noinline)) get_input(int x) {
    return x ^ 0x55AA;
}

int __attribute__((noinline)) use_result(int x) {
    return x * 3 + 1;
}

/* Test function 1: MIPS target with simple jump pattern */
#ifdef __mips__
__attribute__((target("arch=mips32")))
#endif
int test_mips_delay_slot(int a, int b) {
    volatile int cond = get_input(a);
    int temp1 = a + 5;
    int temp2 = b * 2;
    int temp3 = 0;
    
    /* Create a simple conditional jump to label */
    if (cond > 0 && a != b) {
        goto target_label_1;
    }
    
    /* Some other code to create CFG complexity */
    for (int i = 0; i < 3; i++) {
        temp1 += i;
    }
    
    /* Jump target with safe instruction immediately after */
target_label_1:
    /* Safe, non-jump, non-trapping instruction using independent temp */
    temp3 = temp2 & 0xFF;  /* Simple bitwise operation */
    
    /* Use result to prevent elimination */
    return use_result(temp3) + temp1;
}

/* Test function 2: SPARC target with different pattern */
#ifdef __sparc__
__attribute__((target("arch=sparc")))
#endif
int test_sparc_delay_slot(int x, int y) {
    volatile int flag = get_input(x);
    int local_a = x * 3;
    int local_b = y + 7;
    int local_c = 0;
    int local_d = 100;
    
    /* Another simple conditional jump */
    if (flag != 0 && x < y) {
        goto sparc_target;
    }
    
    /* Additional basic blocks */
    if (x > 0) {
        local_a -= x;
    } else {
        local_b += y;
    }
    
    /* Jump target */
sparc_target:
    /* Safe arithmetic on independent variables */
    local_c = local_a - local_b;  /* Simple subtraction */
    
    /* Use in computation */
    return use_result(local_c) ^ local_d;
}

/* Test function 3: Generic pattern with multiple temporaries */
int test_generic_delay(int p, int q) {
    volatile int check = get_input(p);
    int t1 = p + q;
    int t2 = p - q;
    int t3 = p * 2;
    int t4 = 0;
    int t5 = 0;
    
    /* Initialize more temporaries */
    t4 = t1 | t2;
    t5 = t3 ^ t4;
    
    /* Conditional jump */
    if (check % 2 == 0 && p > q) {
        goto generic_target;
    }
    
    /* Loop to create scheduling opportunities */
    for (int j = 0; j < 2; j++) {
        t1 += j;
        t2 -= j;
    }
    
generic_target:
    /* Safe logical operation on independent temps */
    t5 = t4 & 0x7F;  /* Mask operation - cannot trap */
    
    /* Return computation using the result */
    return (t5 << 2) + t1;
}

/* Test function 4: Pattern with more complex condition */
int test_complex_condition(int a, int b, int c) {
    volatile int v1 = get_input(a);
    volatile int v2 = get_input(b);
    int r1 = a + b;
    int r2 = b + c;
    int r3 = a + c;
    int result = 0;
    
    /* More complex condition still producing simple jump */
    if ((v1 > v2) && (a != 0) && (b != 0) && (c != 0)) {
        goto final_target;
    }
    
    /* Alternative path */
    r1 = r1 * 2;
    r2 = r2 / 2;  /* Careful: division but only if we don't take this path */
    
final_target:
    /* Very safe instruction: assignment and increment */
    result = r3 + 1;  /* Cannot trap */
    
    return use_result(result);
}

/* Test function 5: Nested control flow with target label */
int test_nested_pattern(int base) {
    volatile int selector = get_input(base);
    int accum = base;
    int tmp1 = base * 3;
    int tmp2 = base + 10;
    int tmp3 = 0;
    
    /* Outer if */
    if (selector > 100) {
        /* Inner if with goto */
        if (selector < 200) {
            goto inner_target;
        }
        tmp1 += 5;
    }
    
    /* Another basic block */
    for (int k = 0; k < 4; k++) {
        accum += k;
    }
    
inner_target:
    /* Safe shift operation */
    tmp3 = tmp2 << 1;  /* Left shift - always safe */
    
    return accum + tmp3 + tmp1;
}

int main() {
    int result = 0;
    int test_cases[][3] = {
        {10, 20, 30},
        {5, 5, 5},
        {100, 50, 75},
        {0, 1, 2},
        {255, 128, 64}
    };
    
    printf("Testing delay slot filling patterns...\n");
    
    /* Run all test functions with various inputs */
    for (int i = 0; i < 5; i++) {
        int a = test_cases[i][0];
        int b = test_cases[i][1];
        int c = test_cases[i][2];
        
        result ^= test_mips_delay_slot(a, b);
        result ^= test_sparc_delay_slot(b, c);
        result ^= test_generic_delay(a, c);
        result ^= test_complex_condition(a, b, c);
        result ^= test_nested_pattern(a + i);
        
        global_accumulator += result;
    }
    
    printf("Final checksum: %d\n", result);
    printf("Global accumulator: %d\n", global_accumulator);
    
    /* Verify we got non-zero results */
    if (result != 0 && global_accumulator != 0) {
        printf("Test completed successfully.\n");
        return 0;
    } else {
        printf("Warning: All results zero - possible over-optimization.\n");
        return 1;
    }
}
