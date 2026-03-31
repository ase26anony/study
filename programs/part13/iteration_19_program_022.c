/* Test program for GCC auto-profile.cc coverage */
/* Compile with: gcc -O2 -fauto-profile -fprofile-arcs -ftree-vectorize -o test test.c */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/* Volatile to prevent optimization of copy chains */
static volatile int dummy_volatile;

/* Helper to create copy chains */
static inline int copy_chain(int x) {
    int a = x;
    dummy_volatile = a;  /* Prevent optimization */
    int b = a;
    int c = b;
    dummy_volatile = c;
    return c;
}

/* Function 1: Loop-carried phi with copy chain */
int test_phi_after_loop(int n) {
    int result = 0;
    int flag = 0;
    
    /* Loop creates phi for flag at loop header */
    for (int i = 0; i < n; i++) {
        if (i % 3 == 0) {
            flag = 1;  /* One incoming value to phi */
        } else {
            flag = 0;  /* Another incoming value to phi */
        }
        result += i;
    }
    
    /* flag is defined by phi at block entry after loop */
    int flag_copy = copy_chain(flag);  /* Create copy chain */
    
    /* Critical conditional: compare phi-defined variable against 0/1 */
    if (flag_copy == 1) {  /* Comparison against constant 1 */
        result += 1000;
    }
    
    return result;
}

/* Function 2: Multiple returns creating phi for return value */
int test_phi_from_multiple_returns(int x, int y) {
    int condition;
    
    if (x > y) {
        condition = 1;
    } else if (x < y) {
        condition = 0;
    } else {
        condition = 1;  /* Same value from different paths */
    }
    
    /* condition is defined by phi at merge point */
    int cond_copy1 = condition;
    int cond_copy2 = cond_copy1;  /* Simple copy chain */
    dummy_volatile = cond_copy2;
    
    /* Compare against 0 */
    if (cond_copy2 == 0) {
        return x * 2;
    }
    
    return y * 3;
}

/* Function 3: Switch statement creating phi */
int test_phi_from_switch(int mode) {
    int status = 0;
    
    switch (mode % 4) {
        case 0:
            status = 1;
            break;
        case 1:
            status = 0;
            break;
        case 2:
            status = 1;  /* Same as case 0 */
            break;
        default:
            status = 0;  /* Same as case 1 */
            break;
    }
    
    /* status is defined by phi after switch */
    int s1 = status;
    int s2 = s1;
    int s3 = copy_chain(s2);  /* Longer copy chain */
    
    /* Boolean context comparison (implicit != 0) */
    if (s3) {  /* Equivalent to s3 != 0 */
        return 100;
    }
    
    return 200;
}

/* Function 4: Recursive function with phi for condition */
int test_phi_from_recursion(int depth, int max_depth) {
    static int counter = 0;
    
    if (depth >= max_depth) {
        return 0;
    }
    
    int child_result = test_phi_from_recursion(depth + 1, max_depth);
    
    /* Create phi for condition based on recursion depth */
    int condition = (depth % 2 == 0) ? 1 : 0;
    
    /* Copy chain */
    int c1 = condition;
    dummy_volatile = c1;
    int c2 = c1;
    
    /* Compare against 1 */
    if (c2 == 1) {
        counter += depth;
    }
    
    return child_result + depth;
}

/* Function 5: Complex control flow with nested loops */
int test_complex_phi_pattern(int iterations) {
    int sum = 0;
    int hot_flag = 0;
    
    /* Outer loop to make path hot */
    for (int i = 0; i < iterations; i++) {
        int inner_flag = 0;
        
        /* Inner loop with multiple exits */
        for (int j = 0; j < 10; j++) {
            if (j == i % 10) {
                inner_flag = 1;
                break;
            }
            inner_flag = 0;
        }
        
        /* inner_flag is defined by phi after inner loop */
        int flag_copy = inner_flag;
        for (int k = 0; k < 2; k++) {
            flag_copy = flag_copy;  /* Identity assignment creating SSA copies */
        }
        
        /* Hot conditional - likely to be annotated */
        if (flag_copy == 1) {
            sum += i;
            hot_flag = 1;
        } else {
            sum -= i;
            hot_flag = 0;
        }
        
        /* Another use of phi-defined variable */
        int hot_copy = hot_flag;
        dummy_volatile = hot_copy;
        
        if (hot_copy == 0) {
            sum += 5;
        }
    }
    
    return sum;
}

/* Function 6: Use __builtin_expect to hint hot path */
int test_builtin_expect_annotation(int n) {
    int value = 0;
    int decision = 0;
    
    for (int i = 0; i < n; i++) {
        if (i % 7 == 0) {
            decision = 1;
        } else {
            decision = 0;
        }
        
        /* decision is phi-defined */
        int d1 = decision;
        int d2 = d1;
        int d3 = copy_chain(d2);
        
        /* Use __builtin_expect to mark as hot */
        if (__builtin_expect(d3 == 1, 1)) {  /* Likely true */
            value += i * 2;
        } else {
            value += i;
        }
    }
    
    return value;
}

/* Main function to drive execution and create hot paths */
int main(void) {
    int total = 0;
    
    /* Execute many times to create hot paths */
    for (int iteration = 0; iteration < 100000; iteration++) {
        /* Call each test function multiple times */
        total += test_phi_after_loop(iteration % 100 + 1);
        total += test_phi_from_multiple_returns(iteration, iteration / 2);
        total += test_phi_from_switch(iteration);
        
        if (iteration % 1000 == 0) {
            total += test_phi_from_recursion(0, 5);
        }
        
        total += test_complex_phi_pattern(50);
        total += test_builtin_expect_annotation(20);
    }
    
    printf("Result: %d\n", total);
    return 0;
}
