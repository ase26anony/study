/* Test program for GCC auto-profile.cc uncovered lines 1312-1333
 * Creates patterns where:
 * 1. Conditional branches compare SSA names against 0/1
 * 2. Those SSA names are defined by phi nodes
 * 3. There are copy chains between phi and comparison
 * 4. Basic blocks are annotated as hot via profiling
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

volatile int global_counter = 0;

/* Helper to create copy chains */
static inline int copy1(int x) { return x; }
static inline int copy2(int x) { return x; }
static inline int copy3(int x) { return x; }

/* Pattern 1: Phi from loop-carried dependency */
int test_phi_after_loop(int iterations) {
    int result = 0;
    int flag = 0;
    
    /* Loop creates phi for 'flag' */
    for (int i = 0; i < iterations; ++i) {
        if (i % 2 == 0) {
            flag = 1;  /* Even iterations */
        } else {
            flag = 0;  /* Odd iterations */
        }
        result += i;
    }
    
    /* Create copy chain */
    int a = flag;
    int b = copy1(a);
    int c = copy2(b);
    int d = copy3(c);
    
    /* Conditional comparing phi-defined variable against 0/1 */
    if (d == 1) {  /* Comparison against constant 1 */
        result += 100;
    } else if (d == 0) {  /* Comparison against constant 0 */
        result += 200;
    }
    
    return result;
}

/* Pattern 2: Phi from multiple return paths */
int test_phi_from_multiple_returns(int x, int y) {
    int condition;
    
    if (x > y) {
        condition = 1;
    } else if (x < y) {
        condition = 0;
    } else {
        condition = 1;  /* Equal case */
    }
    
    /* Multiple copy assignments */
    volatile int tmp1 = condition;  /* volatile prevents optimization */
    int tmp2 = tmp1;
    int tmp3 = copy1(tmp2);
    int tmp4 = copy2(tmp3);
    
    /* Conditional with phi-defined variable */
    if (tmp4 == 1) {
        return x * 2;
    } else {
        return y * 3;
    }
}

/* Pattern 3: Phi from switch statement */
int test_phi_from_switch(int mode) {
    int status;
    
    switch (mode % 4) {
        case 0:
            status = 1;
            break;
        case 1:
            status = 0;
            break;
        case 2:
            status = 1;
            break;
        case 3:
            status = 0;
            break;
        default:
            status = 1;
    }
    
    /* Extended copy chain */
    int s1 = status;
    int s2 = s1;
    int s3 = copy1(s2);
    int s4 = copy2(s3);
    int s5 = copy3(s4);
    
    /* Hot conditional - use __builtin_expect to hint hot path */
    if (__builtin_expect(s5 == 1, 1)) {
        return mode * 10;
    } else {
        return mode * 20;
    }
}

/* Pattern 4: Recursive function creating phi for condition */
int test_recursive_phi(int n, int depth) {
    if (depth <= 0) {
        return n;
    }
    
    int next_val;
    if (n % 2 == 0) {
        next_val = test_recursive_phi(n / 2, depth - 1);
    } else {
        next_val = test_recursive_phi(n * 3 + 1, depth - 1);
    }
    
    /* Phi created from recursive calls */
    int val = next_val;
    int val2 = copy1(val);
    int val3 = copy2(val2);
    
    /* Conditional on phi result */
    if (val3 > 1000) {
        return 1;  /* Boolean-like return */
    } else {
        return 0;  /* Boolean-like return */
    }
}

/* Pattern 5: Complex control flow with phi merging */
int test_complex_phi_merge(int x) {
    int flag;
    
    /* Multiple basic blocks merging */
    if (x < 0) {
        for (int i = 0; i < 5; i++) {
            x += i;
        }
        flag = 0;
    } else if (x < 100) {
        int j = x;
        while (j > 0) {
            j--;
        }
        flag = 1;
    } else {
        flag = 0;
    }
    
    /* Copy chain with volatile to prevent optimization */
    volatile int f1 = flag;
    int f2 = f1;
    int f3 = f2;
    int f4 = copy1(f3);
    int f5 = copy2(f4);
    
    /* Critical conditional - should be hot */
    if (f5 == 1) {
        return x * 100;
    } else {
        return x * 200;
    }
}

/* Pattern 6: Boolean variable from phi */
int test_boolean_phi(int a, int b) {
    /* Boolean variable that gets phi from different paths */
    _Bool is_greater;
    
    if (a > b) {
        is_greater = 1;  /* true */
    } else {
        is_greater = 0;  /* false */
    }
    
    /* Boolean in conditional context */
    int b1 = is_greater;
    int b2 = copy1(b1);
    int b3 = copy2(b2);
    
    /* if (bool_var) is equivalent to comparison with 1 */
    if (b3) {  /* This becomes comparison with 1 */
        return a - b;
    } else {
        return b - a;
    }
}

/* Main driver that creates hot execution paths */
int main() {
    int total = 0;
    const int ITERATIONS = 100000;
    
    /* Seed for random but reproducible behavior */
    srand(42);
    
    clock_t start = clock();
    
    /* Hot loop - makes basic blocks likely to be annotated as hot */
    for (int i = 0; i < ITERATIONS; ++i) {
        /* Mix different patterns to exercise various phi formations */
        total += test_phi_after_loop(i % 100);
        
        if (i % 3 == 0) {
            total += test_phi_from_multiple_returns(i, i * 2);
        }
        
        if (i % 5 == 0) {
            total += test_phi_from_switch(i);
        }
        
        if (i % 7 == 0) {
            total += test_recursive_phi(i, 3);
        }
        
        if (i % 11 == 0) {
            total += test_complex_phi_merge(i);
        }
        
        if (i % 13 == 0) {
            total += test_boolean_phi(i, i / 2);
        }
        
        /* Prevent compiler from optimizing everything away */
        global_counter += (total & 1);
    }
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("Total result: %d\n", total);
    printf("Global counter: %d\n", global_counter);
    printf("Time elapsed: %.2f seconds\n", elapsed);
    printf("Iterations: %d\n", ITERATIONS);
    
    return total > 0 ? 0 : 1;
}
