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

/* Volatile helper to prevent optimization */
static volatile int volatile_sink;

/* Helper to create copy chains */
static inline int copy_once(int x) { return x; }
static inline int copy_twice(int x) { int a = x; int b = a; return b; }
static inline int copy_thrice(int x) { 
    int a = x; 
    int b = a; 
    int c = b;
    volatile_sink = c;  /* Prevent optimization */
    return c;
}

/* Pattern 1: Phi from loop-carried dependency */
int test_phi_after_loop(int iterations) {
    int result = 0;
    int flag = 0;  /* This will become a phi node */
    
    /* Loop creates phi for 'flag' */
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
    int b = copy_once(a);
    int c = copy_twice(b);
    int d = copy_thrice(c);
    
    /* Conditional comparing phi-defined variable against 0/1 */
    if (d == 1) {  /* cmp_lhs is SSA_NAME from phi via copy chain */
        result += 1000;
    } else if (d == 0) {  /* Another comparison against 0 */
        result += 2000;
    }
    
    return result;
}

/* Pattern 2: Phi from multiple returns with copy chain */
int test_phi_from_multiple_returns(int x, int y) {
    int condition;
    
    /* Different returns create phi for return value */
    if (x > y) {
        condition = 1;
    } else if (x < y) {
        condition = 0;
    } else {
        condition = 1;  /* Same value from different path */
    }
    
    /* Multiple copy assignments */
    int t1 = condition;
    int t2 = t1;
    int t3 = copy_once(t2);
    int t4 = copy_twice(t3);
    
    /* Compare against 1 */
    if (t4 == 1) {
        return x * 2;
    }
    return y * 3;
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
    int c = b;
    int d = copy_once(c);
    int e = copy_twice(d);
    int f = copy_thrice(e);
    
    /* Multiple comparisons against 0/1 */
    if (f == 0) {
        return mode * 10;
    } else if (f == 1) {
        return mode * 20;
    }
    return mode;
}

/* Pattern 4: Recursive function creating phi */
int test_recursive_phi(int n, int depth) {
    if (depth <= 0) {
        return n > 0 ? 1 : 0;  /* Base case returns 0/1 */
    }
    
    int left = test_recursive_phi(n + 1, depth - 1);
    int right = test_recursive_phi(n - 1, depth - 1);
    
    /* Phi node for result */
    int result = (left > right) ? left : right;
    
    /* Copy chain */
    int a = result;
    int b = copy_once(a);
    int c = b;
    
    /* Compare against 1 */
    if (c == 1) {
        return n * 100;
    } else if (c == 0) {
        return n * 200;
    }
    return n;
}

/* Pattern 5: Phi with volatile to prevent SSA simplification */
int test_phi_with_volatile(int x) {
    int flag;
    
    if (x % 2 == 0) {
        volatile_sink = x;
        flag = 1;
    } else {
        volatile_sink = x * 2;
        flag = 0;
    }
    
    /* Complex copy chain with volatile */
    int a = flag;
    volatile_sink = a;
    int b = a;
    int c = copy_thrice(b);
    volatile_sink = c;
    
    /* Hot path hint */
    if (__builtin_expect(c == 1, 1)) {
        return x * 3;
    }
    return x * 5;
}

/* Pattern 6: Nested loops creating complex phi web */
int test_nested_loop_phi(int limit) {
    int outer_flag = 0;
    int sum = 0;
    
    for (int i = 0; i < limit; ++i) {
        int inner_flag = (i % 2 == 0) ? 1 : 0;
        
        for (int j = 0; j < 10; ++j) {
            if (inner_flag == 1) {
                sum += i + j;
            } else {
                sum += i - j;
            }
            
            /* Update outer_flag based on inner loop */
            if (j == 5) {
                outer_flag = inner_flag;
            }
        }
        
        /* Copy chain from phi in outer loop */
        int a = outer_flag;
        int b = copy_twice(a);
        int c = b;
        
        /* Compare phi result against 0 */
        if (c == 0) {
            sum += 100;
        }
    }
    
    return sum;
}

/* Main driver to create hot execution paths */
int main(void) {
    int total = 0;
    const int iterations = 100000;
    
    /* Seed for randomness */
    srand(time(NULL));
    
    printf("Starting auto-profile pattern tests...\n");
    
    /* Hot loop executing all test patterns many times */
    for (int i = 0; i < iterations; ++i) {
        /* Mix different patterns to create various phi/copy/compare scenarios */
        total += test_phi_after_loop(i % 100 + 1);
        total += test_phi_from_multiple_returns(i, i / 2);
        total += test_phi_from_switch(i);
        
        if (i % 100 == 0) {
            total += test_recursive_phi(i % 10, 3);
        }
        
        total += test_phi_with_volatile(i);
        total += test_nested_loop_phi(10 + (i % 5));
        
        /* Prevent compiler from optimizing everything away */
        if (total > 1000000) {
            total = total % 1000000;
        }
    }
    
    printf("Final result: %d\n", total);
    printf("Test completed - patterns should trigger auto-profile analysis\n");
    
    return 0;
}
