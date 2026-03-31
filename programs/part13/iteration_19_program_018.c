/* Test program for GCC auto-profile.cc uncovered lines 1312-1333
 * Creates patterns where:
 * 1. Conditional branch compares SSA name against 0/1
 * 2. SSA name is defined by phi node
 * 3. Copy chain exists between phi and comparison
 * 4. Basic block is annotated as hot
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Volatile helper to prevent optimization */
static volatile int volatile_sink;

/* Helper to create copy chains */
static inline int copy_once(int x) {
    volatile int y = x;
    return y;
}

static inline int copy_twice(int x) {
    int a = x;
    int b = copy_once(a);
    return b;
}

/* Pattern 1: Phi from loop-carried dependency */
int test_phi_after_loop(int n) {
    int result = 0;
    int flag = 0;
    
    /* Loop creates phi for flag */
    for (int i = 0; i < n; i++) {
        if (i % 3 == 0) {
            flag = 1;
        } else {
            flag = 0;
        }
        result += i;
    }
    
    /* Create copy chain */
    int a = flag;
    int b = copy_once(a);
    int c = copy_twice(b);
    
    /* Conditional comparing phi-defined variable against 0/1 */
    if (c == 1) {  /* This should trigger the uncovered code */
        result += 1000;
    } else if (c == 0) {
        result += 500;
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
        condition = 1;  /* Same value from different path */
    }
    
    /* Copy chain */
    int tmp1 = condition;
    int tmp2 = tmp1;
    volatile_sink = tmp2;  /* Prevent optimization */
    int tmp3 = volatile_sink;
    
    /* Comparison against 0/1 */
    if (tmp3 == 1) {
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
        default:
            status = 0;
            break;
    }
    
    /* Multi-step copy chain */
    int a = status;
    int b = a;
    int c = b;
    int d = copy_once(c);
    int e = copy_twice(d);
    
    /* Hot conditional - use __builtin_expect */
    if (__builtin_expect(e == 1, 1)) {
        return mode + 100;
    } else {
        return mode + 200;
    }
}

/* Pattern 4: Recursive function creating phi */
int test_recursive_phi(int depth, int max_depth) {
    if (depth >= max_depth) {
        return 0;
    }
    
    int child_result = test_recursive_phi(depth + 1, max_depth);
    
    /* Phi created from recursive return values */
    int should_process = (child_result % 2 == 0) ? 1 : 0;
    
    /* Copy chain */
    int v1 = should_process;
    int v2 = v1;
    int v3 = copy_once(v2);
    
    /* Comparison */
    if (v3 == 1) {
        return depth * 10 + 1;
    } else {
        return depth * 10 + 2;
    }
}

/* Pattern 5: Phi with global variable */
static int global_counter = 0;

int test_phi_with_global(int iterations) {
    int local_flag;
    
    for (int i = 0; i < iterations; i++) {
        if (global_counter % 2 == 0) {
            local_flag = 1;
        } else {
            local_flag = 0;
        }
        global_counter++;
    }
    
    /* Extended copy chain */
    int x1 = local_flag;
    int x2 = x1;
    int x3 = x2;
    int x4 = copy_once(x3);
    int x5 = copy_twice(x4);
    volatile_sink = x5;
    int x6 = volatile_sink;
    
    /* Hot path conditional */
    for (int i = 0; i < 10; i++) {
        if (x6 == 1) {
            volatile_sink += i;
        }
    }
    
    return x6;
}

/* Pattern 6: Complex nested control flow */
int test_complex_phi(int n) {
    int flag1 = 0, flag2 = 0;
    
    for (int i = 0; i < n; i++) {
        if (i % 2 == 0) {
            flag1 = 1;
            if (i % 3 == 0) {
                flag2 = 1;
            } else {
                flag2 = 0;
            }
        } else {
            flag1 = 0;
            flag2 = (i % 5 == 0) ? 1 : 0;
        }
    }
    
    /* Phi variable from loop */
    int combined = (flag1 && flag2) ? 1 : 0;
    
    /* Copy chain */
    int chain1 = combined;
    int chain2 = chain1;
    int chain3 = copy_twice(chain2);
    
    /* Multiple comparisons against 0/1 */
    int result = 0;
    if (chain3 == 1) {
        result += 100;
    }
    if (chain3 == 0) {
        result += 50;
    }
    
    return result;
}

/* Main driver to create hot execution paths */
int main() {
    int total = 0;
    const int iterations = 100000;
    
    printf("Starting auto-profile pattern tests...\n");
    
    /* Execute all test functions many times to create hot paths */
    for (int i = 0; i < iterations; i++) {
        /* Pattern 1 - frequently executed */
        total += test_phi_after_loop(i % 100 + 1);
        
        /* Pattern 2 - hot path */
        total += test_phi_from_multiple_returns(i, i / 2);
        
        /* Pattern 3 - switch-based phi */
        if (i % 10 == 0) {  /* Less frequent but still hot */
            total += test_phi_from_switch(i);
        }
        
        /* Pattern 4 - recursive phi */
        if (i % 50 == 0) {
            total += test_recursive_phi(0, 5);
        }
        
        /* Pattern 5 - global variable phi */
        total += test_phi_with_global(10);
        
        /* Pattern 6 - complex phi */
        if (i % 20 == 0) {
            total += test_complex_phi(i % 30 + 1);
        }
    }
    
    printf("Total result: %d\n", total);
    printf("Global counter: %d\n", global_counter);
    
    return 0;
}
