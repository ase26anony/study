/* Test program to trigger delay slot filling logic in reorg.cc */
#include <stdio.h>
#include <stdlib.h>

/* Force optimization level on specific functions */
#pragma GCC optimize ("O2")

/* Test function 1: Simple arithmetic after label */
__attribute__((noinline))
static int test_arithmetic_delay(void) {
    volatile int a = 10, b = 20, c = 0;
    int result = 0;
    
    /* Create a simple jump to label */
    if (a < b) {
        goto target_label1;
    }
    
    /* Dead code to create separation */
    result = a * b;
    
target_label1:
    /* Candidate for delay slot filling: simple arithmetic */
    /* This should be safe, non-trapping, and splittable */
    c = a + b;  /* next_trial candidate */
    
    /* Use result to prevent elimination */
    result += c;
    
    /* Another jump to avoid fall-through issues */
    if (c > 0) {
        goto end1;
    }
    
    result += 100;
    
end1:
    return result;
}

/* Test function 2: Bitwise operations after label */
__attribute__((noinline))
static int test_bitwise_delay(void) {
    volatile int x = 0x55, y = 0xAA;
    int temp = 0;
    int sum = 0;
    
    /* Force a jump */
    if (x != 0) {
        goto bitwise_target;
    }
    
    temp = x ^ y;
    
bitwise_target:
    /* Candidate: bitwise operation - safe and non-trapping */
    temp = x | y;  /* next_trial candidate */
    
    /* Use in computation */
    sum = temp + x;
    
    /* Another conditional to create control flow */
    if (sum > 0xFF) {
        goto bitwise_end;
    }
    
    sum += y;
    
bitwise_end:
    return sum;
}

/* Test function 3: Stack-based memory operation */
__attribute__((noinline))
static int test_memory_delay(void) {
    volatile int arr[4] = {1, 2, 3, 4};
    volatile int idx = 2;
    int val = 0;
    
    /* Simple jump condition */
    if (arr[0] > 0) {
        goto mem_target;
    }
    
    val = arr[1];
    
mem_target:
    /* Candidate: stack memory load - should be safe */
    val = arr[idx];  /* next_trial candidate */
    
    /* Use value */
    int ret = val * 2;
    
    /* Prevent tail optimization */
    if (ret < 10) {
        goto mem_end;
    }
    
    ret -= 5;
    
mem_end:
    return ret;
}

/* Test function 4: Comparison operation */
__attribute__((noinline))
static int test_compare_delay(void) {
    volatile int p = 100, q = 200;
    int cmp_result = 0;
    
    /* Unconditional jump */
    goto compare_target;
    
    /* Dead code */
    cmp_result = p - q;
    
compare_target:
    /* Candidate: comparison operation */
    cmp_result = (p < q);  /* next_trial candidate */
    
    /* Use result */
    int score = cmp_result ? 50 : 100;
    
    /* Another jump */
    if (score > 75) {
        goto compare_end;
    }
    
    score += 25;
    
compare_end:
    return score;
}

/* Test function 5: Multiple operations in sequence */
__attribute__((noinline))
static int test_multi_delay(void) {
    volatile int counter = 0;
    volatile int limit = 5;
    int total = 0;
    
    /* Loop with internal goto to increase optimization opportunities */
    for (int i = 0; i < limit; i++) {
        if (i % 2 == 0) {
            goto loop_target;
        }
        
        total += i * 2;
        continue;
        
    loop_target:
        /* Candidate: simple increment */
        counter++;  /* next_trial candidate */
        
        total += i + counter;
    }
    
    return total;
}

/* Test function 6: Register move pattern */
__attribute__((noinline))
static int test_move_delay(void) {
    volatile int src = 42;
    int dst1 = 0, dst2 = 0;
    
    /* Conditional jump */
    if (src != 0) {
        goto move_target;
    }
    
    dst1 = src + 1;
    
move_target:
    /* Candidate: register-to-register move */
    dst2 = src;  /* next_trial candidate */
    
    /* Use both values */
    return dst1 + dst2 * 2;
}

/* Test function 7: Avoid resource conflicts */
__attribute__((noinline))
static int test_no_conflict_delay(void) {
    /* Use distinct variables to avoid resource conflicts */
    volatile int jump_var = 10;      /* Used in jump condition only */
    volatile int safe_var1 = 20;     /* Used after label only */
    volatile int safe_var2 = 30;     /* Another distinct variable */
    int result = 0;
    
    /* Jump based on jump_var only */
    if (jump_var > 5) {
        goto safe_target;
    }
    
    result = jump_var * 2;
    
safe_target:
    /* Candidate: uses variables not involved in jump condition */
    /* This minimizes resource conflicts with &set and &needed */
    safe_var1 = safe_var2 + 5;  /* next_trial candidate */
    
    result += safe_var1;
    return result;
}

/* Test function 8: Nested jumps */
__attribute__((noinline))
static int test_nested_delay(void) {
    volatile int a = 1, b = 2, c = 3;
    int res = 0;
    
    if (a) {
        if (b) {
            goto outer_label;
        }
        res = a + b;
    }
    
    res = c * 2;
    
outer_label:
    /* Candidate: safe arithmetic with distinct vars */
    c = a + b;  /* next_trial candidate */
    
    if (c) {
        goto inner_label;
    }
    
    res += 10;
    
inner_label:
    res += c;
    return res;
}

/* Main function to execute all tests */
int main(void) {
    int total = 0;
    
    printf("Testing delay slot filling patterns...\n");
    
    /* Run all test functions and accumulate results */
    total += test_arithmetic_delay();
    total += test_bitwise_delay();
    total += test_memory_delay();
    total += test_compare_delay();
    total += test_multi_delay();
    total += test_move_delay();
    total += test_no_conflict_delay();
    total += test_nested_delay();
    
    printf("Total result: %d\n", total);
    printf("(This ensures all code paths are executed)\n");
    
    /* Use result to prevent dead code elimination */
    if (total > 0) {
        return 0;
    } else {
        return 1;
    }
}
