/* Test program for GCC auto-profile.cc lines 1312-1333
 * Creates phi-node-defined condition variables with copy chains
 * and constant comparisons (0/1) in hot basic blocks.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Volatile helper to prevent optimization */
static volatile int volatile_sink;

/* Function to create copy chains */
static inline int copy1(int x) { return x; }
static inline int copy2(int x) { return copy1(x); }
static inline int copy3(int x) { return copy2(x); }

/* Pattern 1: Phi from loop-carried dependency */
int test_phi_after_loop(int iterations) {
    int result = 0;
    int flag = 0;  /* Will become phi node at loop header */
    
    /* Loop creates phi for 'flag' at entry */
    for (int i = 0; i < iterations; ++i) {
        if (i % 3 == 0) {
            flag = 1;  /* One incoming value to phi */
        } else {
            flag = 0;  /* Another incoming value to phi */
        }
        result += i * flag;  /* Use flag to prevent dead code */
    }
    
    /* Create copy chain from phi-defined variable */
    int a = flag;      /* First copy */
    int b = copy1(a);  /* Second copy via function */
    int c = copy2(b);  /* Third copy */
    int d = copy3(c);  /* Fourth copy */
    
    /* Conditional comparing phi-defined variable against 0/1 */
    if (d == 1) {  /* Comparison against constant 1 */
        result += 1000;
    } else if (d == 0) {  /* Comparison against constant 0 */
        result += 500;
    }
    
    volatile_sink = result;  /* Prevent optimization */
    return result;
}

/* Pattern 2: Phi from multiple return paths */
int test_phi_from_multiple_returns(int x, int y) {
    int condition;
    
    /* Multiple returns create phi for return value */
    if (x > y) {
        condition = 1;
    } else if (x < y) {
        condition = 0;
    } else {
        condition = 1;  /* Same value from different path */
    }
    
    /* Copy chain */
    int t1 = condition;
    int t2 = t1;
    int t3 = copy1(t2);
    int t4 = copy2(t3);
    
    /* Conditional with phi-defined variable */
    if (t4 == 0) {  /* Compare against 0 */
        return x * 2;
    } else {  /* Implicitly t4 == 1 */
        return y * 3;
    }
}

/* Pattern 3: Phi from switch statement */
int test_phi_from_switch(int mode) {
    int flag;
    
    /* Switch creates phi for flag */
    switch (mode % 4) {
        case 0:
            flag = 1;
            break;
        case 1:
            flag = 0;
            break;
        case 2:
            flag = 1;  /* Same value from different case */
            break;
        default:
            flag = 0;
            break;
    }
    
    /* Longer copy chain */
    int v1 = flag;
    int v2 = v1;
    int v3 = copy1(v2);
    int v4 = copy2(v3);
    int v5 = copy3(v4);
    int v6 = v5;
    
    /* Multiple comparisons against 0/1 */
    if (v6 == 1) {
        return mode * 10;
    }
    if (v6 == 0) {  /* Second comparison in same block */
        return mode * 20;
    }
    
    return mode;
}

/* Pattern 4: Nested loops with phi propagation */
int test_nested_loops_phi(int limit) {
    int outer_flag = 0;
    int sum = 0;
    
    for (int i = 0; i < limit; ++i) {
        int inner_flag = (i % 2 == 0) ? 1 : 0;
        
        for (int j = 0; j < 10; ++j) {
            /* Use inner_flag to create phi in inner loop */
            sum += j * inner_flag;
        }
        
        /* Phi for outer_flag from loop */
        if (i % 3 == 0) {
            outer_flag = 1;
        } else {
            outer_flag = 0;
        }
    }
    
    /* Copy chain from outer_flag phi */
    int f1 = outer_flag;
    int f2 = f1;
    int f3 = copy1(f2);
    
    /* Comparison against 0/1 */
    if (f3 != 0) {  /* != 0 is equivalent to == 1 for 0/1 values */
        sum += 10000;
    } else {
        sum += 20000;
    }
    
    return sum;
}

/* Pattern 5: Recursive function creating phi */
int test_recursive_phi(int n, int depth) {
    if (depth <= 0) {
        return (n % 2 == 0) ? 1 : 0;
    }
    
    int left = test_recursive_phi(n * 3 + 1, depth - 1);
    int right = test_recursive_phi(n / 2, depth - 1);
    
    /* Phi created from recursive calls */
    int combined = (left == 1 || right == 1) ? 1 : 0;
    
    /* Copy chain */
    int c1 = combined;
    int c2 = copy1(c1);
    int c3 = copy2(c2);
    
    /* Comparison */
    if (c3 == 1) {
        return n + 100;
    } else {
        return n + 200;
    }
}

/* Pattern 6: Boolean variable with explicit 0/1 */
int test_bool_phi(int a, int b) {
    /* Boolean operations produce 0/1 results */
    _Bool flag1 = (a > b);
    _Bool flag2 = (a != b);
    
    /* Phi created from conditional */
    _Bool result = flag1 ? flag2 : 0;
    
    /* Copy chain with bool */
    _Bool b1 = result;
    _Bool b2 = b1;
    int b3 = b2;  /* Convert to int for comparison */
    int b4 = copy1(b3);
    
    /* Comparison - bool is guaranteed 0/1 */
    if (b4) {  /* Implicit comparison against 0 */
        return a * b;
    }
    
    return a + b;
}

/* Main driver to create hot execution paths */
int main(void) {
    clock_t start = clock();
    int total_result = 0;
    const int hot_iterations = 100000;
    
    printf("Starting auto-profile pattern tests...\n");
    
    /* Hot loop to make basic blocks "annotated" as hot */
    for (int i = 0; i < hot_iterations; ++i) {
        /* Mix different patterns to exercise various phi structures */
        total_result += test_phi_after_loop(i % 100 + 1);
        total_result += test_phi_from_multiple_returns(i, i / 2);
        total_result += test_phi_from_switch(i);
        total_result += test_nested_loops_phi(i % 50 + 1);
        
        if (i % 1000 == 0) {
            total_result += test_recursive_phi(i, 3);
            total_result += test_bool_phi(i, i * 2);
        }
        
        /* Prevent loop unrolling from simplifying too much */
        volatile_sink = i;
    }
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("Total result: %d\n", total_result);
    printf("Elapsed time: %.2f seconds\n", elapsed);
    printf("Iterations: %d\n", hot_iterations);
    
    return total_result != 0 ? 0 : 1;
}
