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
volatile int sink = 0;

/* Helper to create copy chains */
static inline int copy1(int x) { volatile int t = x; return t; }
static inline int copy2(int x) { volatile int t = x; return t; }
static inline int copy3(int x) { volatile int t = x; return t; }

/* Pattern 1: Phi from loop-carried dependency */
int test_phi_after_loop(int iterations) {
    int result = 0;
    int flag = 0;
    
    /* Loop creates phi for 'flag' at entry */
    for (int i = 0; i < iterations; ++i) {
        if (i % 3 == 0) {
            flag = 1;  /* One incoming value to phi */
        } else {
            flag = 0;  /* Another incoming value to phi */
        }
        result += i;
    }
    
    /* Create copy chain from phi-defined variable */
    int a = flag;
    int b = copy1(a);
    int c = copy2(b);
    int d = copy3(c);
    
    /* Conditional comparing phi-defined variable against 0/1 */
    if (d == 1) {  /* Comparison against constant 1 */
        result += 1000;
    } else if (d == 0) {  /* Comparison against constant 0 */
        result += 2000;
    }
    
    return result;
}

/* Pattern 2: Phi from multiple return paths */
int test_phi_from_multiple_returns(int x, int y) {
    int condition;
    
    /* Multiple control flow paths create phi for 'condition' */
    if (x > y) {
        condition = 1;
    } else if (x < y) {
        condition = 0;
    } else {
        condition = 1;  /* Same value from different path */
    }
    
    /* Copy chain */
    volatile int t1 = condition;
    int t2 = t1;
    volatile int t3 = t2;
    int t4 = t3;
    
    /* Conditional with copy chain */
    if (t4 == 0) {
        return x * 2;
    } else {
        return y * 3;
    }
}

/* Pattern 3: Phi from switch statement */
int test_phi_from_switch(int mode) {
    int flag;
    
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
    int a = flag;
    int b = a;
    volatile int c = b;
    int d = c;
    volatile int e = d;
    int f = e;
    
    /* Comparison against 0/1 */
    if (f == 1) {
        return mode * 10;
    } else {
        return mode * 20;
    }
}

/* Pattern 4: Nested loops with phi propagation */
int test_nested_loops_phi(int limit) {
    int outer_flag = 0;
    int sum = 0;
    
    for (int i = 0; i < limit; ++i) {
        int inner_flag = (i % 2 == 0) ? 1 : 0;
        
        for (int j = 0; j < 10; ++j) {
            /* Create phi in inner loop */
            if (j % 3 == 0) {
                inner_flag = 1 - inner_flag;  /* Flip the flag */
            }
            sum += j;
        }
        
        /* Phi from inner loop to outer scope */
        outer_flag = inner_flag;
        
        /* Copy and test */
        int tmp = outer_flag;
        volatile int vtmp = tmp;
        
        if (vtmp == 0) {
            sum += i;
        } else if (vtmp == 1) {
            sum -= i;
        }
    }
    
    return sum;
}

/* Pattern 5: Recursive function creating phi for condition */
int test_recursive_phi(int n, int depth) {
    if (depth <= 0) {
        return n;
    }
    
    int next_val;
    if (n % 2 == 0) {
        next_val = test_recursive_phi(n / 2, depth - 1);
    } else {
        next_val = test_recursive_phi(3 * n + 1, depth - 1);
    }
    
    /* Phi from recursive calls */
    int flag = (next_val > 100) ? 1 : 0;
    
    /* Copy chain */
    int a = flag;
    volatile int b = a;
    int c = b;
    
    /* Comparison */
    if (c == 1) {
        return next_val * 2;
    } else {
        return next_val / 2;
    }
}

/* Pattern 6: Boolean variable with explicit 0/1 comparison */
int test_bool_phi(int a, int b) {
    /* Boolean variable that becomes phi */
    _Bool flag = (a > b);
    
    /* Multiple assignments to create phi */
    for (int i = 0; i < 5; ++i) {
        if (i % 2 == 0) {
            flag = (a < b);  /* Different incoming value */
        }
    }
    
    /* Copy through volatile */
    volatile _Bool vflag = flag;
    _Bool flag2 = vflag;
    volatile _Bool vflag2 = flag2;
    
    /* Explicit comparison against 0/1 (bool is 0 or 1) */
    if (vflag2 == 1) {
        return a + b;
    } else if (vflag2 == 0) {
        return a - b;
    }
    return 0;
}

/* Pattern 7: Complex control flow with merge points */
int test_complex_control_flow(int x) {
    int value;
    
    if (x < 0) {
        for (int i = 0; i < 3; ++i) {
            value = 0;
        }
    } else if (x > 100) {
        while (x > 50) {
            x--;
            value = 1;
        }
    } else {
        do {
            value = x % 2;
            x /= 2;
        } while (x > 0);
    }
    
    /* Multiple incoming edges create phi for 'value' */
    int a = value;
    volatile int b = a;
    int c = b;
    
    /* Hot comparison (use __builtin_expect) */
    if (__builtin_expect(c == 0, 1)) {
        return x * 100;
    } else if (__builtin_expect(c == 1, 0)) {
        return x * 200;
    }
    
    return x;
}

/* Main driver that creates hot execution paths */
int main() {
    int total = 0;
    const int ITERATIONS = 100000;
    
    /* Seed for reproducibility */
    srand(42);
    
    clock_t start = clock();
    
    /* Hot loop to make basic blocks annotated as hot */
    for (int i = 0; i < ITERATIONS; ++i) {
        /* Call each test function to exercise different patterns */
        total += test_phi_after_loop(i % 100 + 1);
        total += test_phi_from_multiple_returns(i, i / 2);
        total += test_phi_from_switch(i);
        total += test_nested_loops_phi(i % 50 + 1);
        total += test_recursive_phi(i, 3);
        total += test_bool_phi(i, i * 2);
        total += test_complex_control_flow(i);
        
        /* Prevent optimization */
        sink = total;
    }
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("Total: %d\n", total);
    printf("Time: %.2f seconds\n", elapsed);
    printf("Iterations: %d\n", ITERATIONS);
    
    return total != 0 ? 0 : 1;
}
