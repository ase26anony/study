/* Test program for triggering delay slot filling logic in reorg.cc */
#include <stdio.h>
#include <stdlib.h>

/* Force optimization on specific functions */
#ifdef __GNUC__
#define OPTIMIZE_O2 __attribute__((optimize("O2")))
#else
#define OPTIMIZE_O2
#endif

/* Avoid dead code elimination */
volatile int g_volatile = 0;

/* Test 1: Simple arithmetic after label */
OPTIMIZE_O2
static int test1(void) {
    int a = 10, b = 20, c = 30;
    int result = 0;
    
    /* Create a simple jump to label */
    if (a < b) {
        goto target_label1;
    }
    
    /* Unreachable code to create fall-through path */
    result += 1000;
    
target_label1:
    /* Candidate for next_trial: simple arithmetic */
    /* This should be safe to move into delay slot */
    c = a + b;  /* Simple add, no side effects */
    
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

/* Test 2: Bitwise operations with stack variables */
OPTIMIZE_O2
static int test2(void) {
    int x = 5, y = 3, z = 7;
    int temp = 0;
    
    /* Loop to encourage optimization */
    for (int i = 0; i < 2; i++) {
        if (x > y) {
            goto compute_label;
        }
        temp += i;
    }
    
    /* Different operation to avoid pattern matching */
    z = x | y;
    return temp + z;

compute_label:
    /* Candidate: bitwise operation */
    /* Should pass resource checks with distinct variables */
    z = x & y;  /* AND operation, no traps */
    
    /* Use in computation */
    temp = z * 2;
    
    /* Jump to return */
    goto finish;

finish:
    return temp + x + y;
}

/* Test 3: Safe memory operation (stack load/store) */
OPTIMIZE_O2
static int test3(void) {
    int arr[4] = {1, 2, 3, 4};
    int idx = 0;
    int sum = 0;
    
    /* Conditional jump */
    if (arr[0] == 1) {
        goto process_label;
    }
    
    sum = 100;
    return sum;

process_label:
    /* Candidate: stack memory load (should be safe) */
    /* Not a volatile, not through pointer - unlikely to trap */
    int val = arr[1];  /* Load from stack array */
    
    /* Simple arithmetic with loaded value */
    sum = val * 2;
    
    /* Another conditional to create control flow */
    if (sum > 0) {
        goto done_label;
    }
    
    sum += 10;

done_label:
    return sum + idx;
}

/* Test 4: Comparison operation */
OPTIMIZE_O2
static int test4(void) {
    int p = 50, q = 60, r = 70;
    int flag = 0;
    
    /* Unconditional goto to create simple jump */
    if (p != 0) {
        goto compare_label;
    }
    
    r = p + q;
    return r;

compare_label:
    /* Candidate: comparison operation */
    /* Sets condition codes but no memory side effects */
    flag = (p < q);  /* Comparison, can be moved */
    
    /* Use flag to compute result */
    int result = flag ? p : q;
    
    /* Prevent tail optimization */
    g_volatile = result;
    
    return result + r;
}

/* Test 5: Multiple operations with register moves */
OPTIMIZE_O2
static int test5(void) {
    register int r1 asm("t0") = 100;
    register int r2 asm("t1") = 200;
    int output = 0;
    
    /* Nested condition to create jump */
    if (r1 > 50) {
        if (r2 < 300) {
            goto operation_label;
        }
    }
    
    output = r1 - r2;
    return output;

operation_label:
    /* Candidate: register-to-register operation */
    /* try_split should handle this */
    r1 = r2 + 10;  /* Simple add with immediate */
    
    /* Chain of simple operations */
    output = r1;
    output += 5;
    
    /* Final jump */
    goto exit_label;

exit_label:
    return output;
}

/* Test 6: Mixed operations in loop */
OPTIMIZE_O2
static int test6(void) {
    int counter = 0;
    int accum = 0;
    
    while (counter < 3) {
        /* Create label target for jump */
        if (counter == 1) {
            goto loop_label;
        }
        
        accum += counter;
        counter++;
        continue;
        
    loop_label:
        /* Candidate: shift operation */
        /* Should be eligible for delay slot */
        accum = accum << 1;  /* Shift left, no traps */
        
        counter++;
        /* Jump back to loop */
        goto continue_loop;
        
    continue_loop:
        /* Empty to complete the pattern */
        ;
    }
    
    return accum;
}

/* Test 7: Avoid resource conflicts explicitly */
OPTIMIZE_O2
static int test7(void) {
    /* Use completely distinct variable sets */
    int jump_var = 42;
    int delay_var1 = 100;
    int delay_var2 = 200;
    int result_var = 0;
    
    /* The jump condition uses only jump_var */
    if (jump_var > 0) {
        goto safe_delay;
    }
    
    result_var = 999;
    return result_var;

safe_delay:
    /* Candidate: uses only delay_var1, delay_var2 */
    /* No overlap with jump condition variables */
    result_var = delay_var1 - delay_var2;
    
    /* Simple use to prevent elimination */
    g_volatile = result_var;
    
    return result_var + jump_var;
}

/* Main function to execute all tests */
int main(void) {
    int total = 0;
    
    /* Run all test functions */
    total += test1();
    total += test2();
    total += test3();
    total += test4();
    total += test5();
    total += test6();
    total += test7();
    
    /* Print result to ensure execution */
    printf("Total: %d\n", total);
    
    /* Also use volatile to prevent optimization */
    g_volatile = total;
    
    return (total > 0) ? 0 : 1;
}
