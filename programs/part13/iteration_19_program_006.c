/* Test program for GCC auto-profile.cc uncovered lines 1312-1333
 * Creates phi-node-defined condition variables with copy chains
 * and constant 0/1 comparisons in hot basic blocks.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Volatile helper to prevent optimization */
static volatile int volatile_sink;

/* Function to create copy chains */
static inline int copy_once(int x) {
    volatile_sink = x;
    return x;
}

static inline int copy_twice(int x) {
    int a = copy_once(x);
    int b = copy_once(a);
    return b;
}

static inline int copy_chain(int x) {
    int a = x;
    int b = a;
    int c = b;
    int d = copy_once(c);
    int e = copy_twice(d);
    return e;
}

/* Pattern 1: Phi from loop-carried dependency */
int test_phi_after_loop(int iterations) {
    int result = 0;
    int flag = 0;  /* This will become a phi node */
    
    for (int i = 0; i < iterations; i++) {
        /* Loop creates phi for 'flag' at entry */
        if (i % 3 == 0) {
            flag = 1;
        } else if (i % 7 == 0) {
            flag = 0;
        }
        /* Some computation to prevent optimization */
        result += i * i;
    }
    
    /* Create copy chain from phi-defined variable */
    int a = flag;
    int b = copy_once(a);
    int c = copy_twice(b);
    int d = copy_chain(c);
    
    /* Conditional with constant 0/1 comparison */
    if (d == 0) {  /* cmp_lhs is SSA_NAME from phi via copy chain */
        result += 100;
    } else if (d == 1) {  /* Another constant 1 comparison */
        result += 200;
    }
    
    return result;
}

/* Pattern 2: Phi from multiple return paths */
int test_phi_from_multiple_returns(int x, int y) {
    int condition;
    
    if (x > 100) {
        condition = 1;
    } else if (x < -100) {
        condition = 0;
    } else {
        condition = (y % 2);
    }
    
    /* Multiple copy assignments */
    int tmp1 = condition;
    int tmp2 = tmp1;
    int tmp3 = copy_once(tmp2);
    int tmp4 = copy_twice(tmp3);
    
    /* Hot conditional - use __builtin_expect */
    if (__builtin_expect(tmp4 == 1, 1)) {
        return x * 2;
    } else if (__builtin_expect(tmp4 == 0, 0)) {
        return y * 3;
    }
    
    return x + y;
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
            flag = 1;
            break;
        default:
            flag = 0;
            break;
    }
    
    /* Longer copy chain */
    int a = flag;
    int b = a;
    int c = copy_once(b);
    int d = c;
    int e = copy_twice(d);
    int f = e;
    int g = copy_chain(f);
    
    /* Conditional in hot loop */
    int sum = 0;
    for (int i = 0; i < 1000; i++) {
        if (g == 0) {
            sum += i;
        } else if (g == 1) {
            sum += i * 2;
        }
    }
    
    return sum;
}

/* Pattern 4: Recursive function creating phi */
int test_recursive_phi(int n, int depth) {
    if (depth <= 0) {
        return n % 2;  /* Returns 0 or 1 */
    }
    
    int left = test_recursive_phi(n * 3 + 1, depth - 1);
    int right = test_recursive_phi(n / 2, depth - 1);
    
    /* Phi node created from recursive calls */
    int result = (left > right) ? left : right;
    
    /* Copy chain */
    int a = result;
    int b = copy_once(a);
    int c = b;
    
    /* Constant comparison */
    if (c == 0) {
        return 1;
    } else if (c == 1) {
        return 0;
    }
    
    return -1;
}

/* Pattern 5: Global variable with phi */
static int global_counter = 0;

int test_global_phi(int iterations) {
    int local_flag;
    
    /* Global creates phi when updated in loop */
    for (int i = 0; i < iterations; i++) {
        global_counter++;
        if (global_counter % 100 == 0) {
            local_flag = 1;
        } else {
            local_flag = 0;
        }
    }
    
    /* Multiple SSA copies */
    int x = local_flag;
    int y = x;
    int z = copy_once(y);
    int w = copy_twice(z);
    
    /* Hot conditional path */
    int result = 0;
    for (int i = 0; i < 500; i++) {
        if (w == 1) {
            result += i * i;
        } else if (w == 0) {
            result += i;
        }
    }
    
    return result;
}

/* Pattern 6: Boolean variable (guaranteed 0/1) */
int test_bool_phi(int a, int b) {
    /* Boolean operations create 0/1 values */
    _Bool flag1 = (a > b);
    _Bool flag2 = (a % 2 == 0);
    _Bool flag3 = (b % 3 == 0);
    
    /* Phi from ternary */
    _Bool combined = flag1 ? flag2 : flag3;
    
    /* Copy chain with bool */
    _Bool c1 = combined;
    _Bool c2 = c1;
    _Bool c3 = copy_once(c2);
    
    /* if (bool_var) is equivalent to if (bool_var == 1) */
    if (c3) {  /* Should trigger cmp_rhs == 1 */
        return a + b;
    } else {
        return a - b;
    }
}

/* Main driver to create hot execution paths */
int main(void) {
    int total = 0;
    const int iterations = 100000;
    
    /* Seed for variability */
    srand(time(NULL));
    
    printf("Starting auto-profile pattern tests...\n");
    
    /* Hot loop to make basic blocks "annotated" */
    for (int i = 0; i < iterations; i++) {
        /* Call all test functions to create various patterns */
        total += test_phi_after_loop(i % 100 + 1);
        total += test_phi_from_multiple_returns(i, i * 2);
        total += test_phi_from_switch(i);
        
        if (i % 10 == 0) {
            total += test_recursive_phi(i, 3);
        }
        
        total += test_global_phi(10);
        total += test_bool_phi(i, i / 2 + 1);
        
        /* Prevent optimization */
        if (total > 1000000) {
            total %= 1000000;
        }
    }
    
    printf("Final result: %d\n", total);
    printf("Global counter: %d\n", global_counter);
    
    return total != 0 ? 0 : 1;
}
